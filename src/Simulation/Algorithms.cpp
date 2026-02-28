

#include <bit>

#include "Algorithms.h"
#include "Context.h"
#include "Arc.h"
#include "Library.h"
#include "Simulation.h"
#include "Sequential.h"
#include "Template.h"
#include "Utils.h"

namespace open_char {

Algorithms::Algorithms(Context *ctx) :
    ctx_(ctx)
{}

int Algorithms::ToLogic(Volt val)
{
    OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
    if (abs(val - op_cond.GetSupply()->GetVddVoltage()) < 0.01)
        return 1;
    return 0;
}

int Algorithms::GetBit(int64_t v, size_t index)
{
    return (v >> index) & 0x1;
}

void Algorithms::PrepareComboLogicTableAndLeakageSims(Cell &cell)
{
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
    Supply *supply = op_cond.GetSupply();
    Volt log0_v = supply->GetGndVoltage();
    Volt log1_v = supply->GetVddVoltage();

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {

        size_t n_sims = 1;
        for (size_t i = 0; i < cell.GetPinsCount(PinDirection::IN); i++)
            n_sims *= 2;

        for (int64_t i_pin_vect = 0; i_pin_vect < static_cast<int64_t>(n_sims); i_pin_vect++) {

            std::string sim_name = "COMBO_LOGTBLLKG";
            size_t tmp = i_pin_vect;
            for (const auto & i_pin : cell.GetPins(PinDirection::IN)) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, tmp & 0x1);
                tmp >>= 1;
            }

            // TODO: Wrap simulation construction with common stuff!
            Simulation *sim = new Simulation(ctx_, sim_name, &cell, SimulationKind::DC);

            sim->SetTemp(op_cond.GetTemperature());
            sim->SetSupply(op_cond.GetSupply());

            for (const auto & netlist : ctx_->GetNetlists())
                sim->AddInclude(netlist);

            for (const auto & model : ctx_->GetModels())
                sim->AddModel(model);

            int i_pin_index = 0;
            for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
                size_t i_pin_val = ((i_pin_vect >> i_pin_index) & 0x1);
                sim->AddStimuli((Pin*)&i_pin, Stimulus((i_pin_val == 1) ? log1_v : log0_v));

                i_pin_index++;
            }

            sim->PutMetaData(static_cast<int>(i_pin_vect));

            o_pin.AddSimulation(sim);
            ctx_->GetSimulationPool().PushSimulation(sim);
        }
    }
}

void Algorithms::MeasureComboLogicTables(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (Simulation *sim : o_pin.GetSimulations()) {
            Waves w = sim->ReadWaves();
            w.Print();
            int64_t i_pin_vect = sim->GetMetaDataAt(0);
            o_pin.AddLogicTableEntry(i_pin_vect, ToLogic(w.GetVoltage(o_pin.name_)[0]));
        }
    }
}

void Algorithms::MeasureComboLeakage(Cell &cell)
{
    // Logic Table simulations contain all combinations of input pin states so
    // they are used to extract leakage upon each input combination.

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (Simulation *sim : o_pin.GetSimulations()) {

            Waves w = sim->ReadWaves();
            Supply *s = cell.GetLibrary()->GetOpCond().GetSupply();
            NanoWatt lkg = w.GetCurrent(s->GetVddName())[0] * s->GetVddVoltage() * 1E3;

            Expression *e = new Expression(ExpressionKind::CONSTANT, 1);
            int64_t i_pin_vect = sim->GetMetaDataAt(0);

            for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
                Expression *tmp = new Expression(ExpressionKind::TERM, &(i_pin));
                if ((i_pin_vect & 0x1) == 0) {
                    tmp = new Expression(ExpressionKind::NOT, tmp);
                }
                e = new Expression(ExpressionKind::AND, e, tmp);
                i_pin_vect >>= 1;
            }

            e->Simplify();
            cell.AddLeakageTableEntry(e, lkg);
        }
        break;
    }
}

Expression* Algorithms::ComboSumOfProducts(Cell& cell, Pin& o_pin)
{
    Expression *e = new Expression(ExpressionKind::CONSTANT, 0);

    for (auto &lt_entry : o_pin.GetLogicTable()) {
        int output = lt_entry.second;

        Expression* row_e = new Expression(ExpressionKind::CONSTANT, 1);

        if (output == 0)
            continue;

        int64_t inputs = lt_entry.first;

        int i = 0;
        for (auto &i_pin : cell.GetPins(PinDirection::IN)) {
            int val = (inputs >> i) & 0x1;

            Expression *e_rhs = new Expression(ExpressionKind::TERM, &i_pin);
            if (val == 0) {
                e_rhs = new Expression(ExpressionKind::NOT, e_rhs);
            }
            row_e = new Expression(ExpressionKind::AND, row_e, e_rhs);
            i++;
        }

        e = new Expression(ExpressionKind::OR, e, row_e);
    }

    return e;
}

Expression* Algorithms::ComboProductOfSums(Cell& cell, Pin& o_pin)
{
    // TODO: Cross-check this is correct!
    Expression *e = new Expression(ExpressionKind::CONSTANT, 1);

    for (const auto &lt_entry : o_pin.GetLogicTable()) {
        int output = lt_entry.second;

        Expression* row_e = new Expression(ExpressionKind::CONSTANT, 0);

        if (output == 1)
            continue;

        int64_t inputs = lt_entry.first;

        int i = 0;
        for (auto &i_pin : cell.GetPins(PinDirection::IN)) {
            int val = (inputs >> i) & 0x1;

            Expression *e_rhs = new Expression(ExpressionKind::TERM, &i_pin);
            if (val == 1) {
                e_rhs = new Expression(ExpressionKind::NOT, e_rhs);
            }
            row_e = new Expression(ExpressionKind::OR, row_e, e_rhs);
            i++;
        }

        e = new Expression(ExpressionKind::AND, e, row_e);
    }

    return e;
}

Expression* Algorithms::ComboRecognizeXor(Cell& cell, Pin& o_pin)
{
    size_t n_inputs = cell.GetPinsCount(PinDirection::IN);

    // Checks all rows are XOR of inputs
    for (const auto & row : o_pin.GetLogicTable()) {
        int64_t inputs = row.first;
        int output = row.second;

        int cur_out = 0;

        for (size_t i = 0; i < n_inputs; i++) {
            cur_out ^= (inputs >> i) & 0x1;
        }

        if (cur_out != output) {
            return nullptr;
        }
    }

    Expression *e = new Expression(ExpressionKind::CONSTANT, 0);

    for (auto & i_pin : cell.GetPins(PinDirection::IN)) {
        Expression *term = new Expression(ExpressionKind::TERM, &i_pin);
        e = new Expression(ExpressionKind::XOR, e, term);
    }

    return e;
}

void Algorithms::CalculateComboLogicFunctions(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        auto &log_table = o_pin.GetLogicTable();
        assert (log_table.size() > 0);

        Expression *e = ComboRecognizeXor(cell, o_pin);

        if (e == nullptr) {
            size_t n_zeros = 0;
            size_t n_ones = 0;

            for (const auto & row : log_table) {
                assert (row.second == 0 || row.second == 1);
                if (row.second == 1)
                    n_ones++;
                if (row.second == 0)
                    n_zeros++;
            }

            if (n_ones > n_zeros) {
                e = ComboProductOfSums(cell, o_pin);
            } else {
                e = ComboSumOfProducts(cell, o_pin);
            }
        }

        e->Simplify();
        o_pin.SetLogicFunction(e);
        o_pin.PrintLogicFunction();
    }
}

void Algorithms::PrepareComboDelayAndPowerSims(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {

        struct test_vect {
            int64_t     in_i;
            int         out_i;
            int64_t     in_j;
            int         out_j;
        };
        std::vector<test_vect> test_vects;

        // Measure test vectors with output different and only one
        // input different
        int i = 0;
        for (auto & row_i : o_pin.GetLogicTable()) {
            int64_t row_i_in = row_i.first;
            int row_i_out = row_i.second;

            int j = 0;
            for (auto & row_j : o_pin.GetLogicTable()) {
                int64_t row_j_in = row_j.first;
                int row_j_out = row_j.second;

                if (j <= i) {
                    j++;
                    continue;
                }

                if (row_j_out == row_i_out) {
                    j++;
                    continue;
                }

                int64_t diff = row_i_in ^ row_j_in;
                if (std::popcount((unsigned long)diff) != 1) {
                    j++;
                    continue;
                }

                test_vect tv = {
                    .in_i   = row_i_in,
                    .out_i  = row_i_out,
                    .in_j   = row_j_in,
                    .out_j  = row_j_out
                };
                test_vects.push_back(tv);

                j++;
            }
            i++;
        }

        // Simulate all test vectors - Both directions
        for (const auto & tv : test_vects) {
            PrepareComboArcSims(o_pin, tv.in_i, tv.in_j, tv.out_i, tv.out_j);
        }
    }
}

int Algorithms::PrepareComboArcSims(Pin &o_pin, int64_t in_a, int64_t in_b, int out_a, int out_b)
{
    Cell *cell = o_pin.cell_;
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
    Template *templ = cell->GetDelayTemplate();

    o_pin.AddArc(Arc(&o_pin, templ, in_a, in_b, out_a, out_b));
    size_t arc_index = o_pin.GetArcs().size() - 1;

    for (size_t i = 0; i < 2; i++) {
        int64_t i_from_vect = (i == 0) ? in_a  : in_b;
        int64_t i_to_vect   = (i == 0) ? in_b  : in_a;
        int     o_from      = (i == 0) ? out_a : out_b;

        std::string prefix = "COMBO_DLYPWR";
        size_t j = 0;

        for (const auto & i_pin : cell->GetPins(PinDirection::IN)) {
            prefix = sprintf("%s_%s%d%d", prefix, i_pin.name_,
                             GetBit(i_from_vect, j), GetBit(i_to_vect, j));
            j++;
        }

        int i_tran_index = 0;
        for (NanoSecond i_tran : templ->GetIndex1()) {

            int o_cap_index = 0;
            for (const PicoFarad o_cap : templ->GetIndex2()) {

                std::string sim_name = sprintf("%s_TRAN_%f_CAP_%f", prefix, i_tran, o_cap);

                // TODO: Wrap construction to some common tasks
                Simulation *sim = new Simulation(ctx_, sim_name, cell, SimulationKind::TRAN);
                sim->SetTemp(op_cond.GetTemperature());
                sim->SetSupply(op_cond.GetSupply());

                for (const auto & netlist : ctx_->GetNetlists())
                    sim->AddInclude(netlist);

                for (const auto & model : ctx_->GetModels())
                    sim->AddModel(model);

                Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
                Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

                sim->PutMetaData(i_tran_index);
                sim->PutMetaData(o_cap_index);

                int i = 0;
                for (auto & i_pin : cell->GetPins(PinDirection::IN)) {

                    int i_from = GetBit(i_from_vect, i);
                    int i_to = GetBit(i_to_vect, i);

                    if (i_from == i_to) {
                        sim->AddStimuli((Pin*)&i_pin, Stimulus((i_from == 1) ? log1_v : log0_v));
                    } else {

                        assert((i_from == 0 && i_to == 1) | (i_from == 1 && i_to == 0));

                        Variables &vars = ctx_->GetVariables();

                        sim->PutMetaData(i_from);
                        sim->PutMetaData(o_from);

                        NanoSecond i_tran_cor = i_tran;
                        // Assumes linear ramp
                        if (i_from == 0) {
                            double slew_lower_rise = vars.GetDoubleVariable("slew_lower_rise");
                            double slew_upper_rise = vars.GetDoubleVariable("slew_upper_rise");
                            i_tran_cor *= (1 + slew_lower_rise + (1 - slew_upper_rise));
                        } else {
                            double slew_lower_fall = vars.GetDoubleVariable("slew_lower_fall");
                            double slew_upper_fall = vars.GetDoubleVariable("slew_upper_fall");
                            i_tran_cor *= (1 + slew_lower_fall + (1 - slew_upper_fall));
                        }

                        // TODO: Refine
                        Stimulus edge (
                            (i_from == 1) ? log1_v : log0_v,
                            (i_to == 1)   ? log1_v : log0_v,
                            1,
                            i_tran_cor,
                            i_tran_cor,
                            100,
                            100,
                            1
                        );
                        sim->AddStimuli((Pin*)&i_pin, std::move(edge));
                    }
                    i++;
                }

                sim->AddLoad(&o_pin, o_cap);

                o_pin.GetArcs()[arc_index].AddSimulation(sim);
                ctx_->GetSimulationPool().PushSimulation(sim);

                o_cap_index++;
            }
            i_tran_index++;
        }
    }

    return 0;
}

void Algorithms::MeasureComboDelays(Cell &cell)
{
    assert (cell.GetKind() == CellKind::COMBINATIONAL);

    Variables &vars = ctx_->GetVariables();

    double delay_in_rise = vars.GetDoubleVariable("delay_in_rise");
    double delay_in_fall = vars.GetDoubleVariable("delay_in_fall");
    double delay_out_rise = vars.GetDoubleVariable("delay_out_rise");
    double delay_out_fall = vars.GetDoubleVariable("delay_out_fall");

    Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
    Volt vdd_voltage = supply->GetVddVoltage();
    std::string vdd_name = supply->GetVddName();

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {
            for (Simulation *sim : arc.GetSimulations()) {

                Waves w = sim->ReadWaves();
                Pin *related_pin = arc.GetRelatedPin();

                int i_tran_index = sim->GetMetaDataAt(0);
                int o_cap_index = sim->GetMetaDataAt(1);
                int i_from = sim->GetMetaDataAt(2);
                int o_from = sim->GetMetaDataAt(3);

                double i_th = (i_from == 0) ? delay_in_rise : delay_in_fall;
                double o_th = (o_from == 0) ? delay_out_rise : delay_out_fall;

                i_th *= vdd_voltage;
                o_th *= vdd_voltage;

                NanoSecond i_edge = w.FindTransitionTime(related_pin->name_, i_from, i_th);
                NanoSecond o_edge = w.FindTransitionTime(o_pin.name_, o_from, o_th);

                NanoSecond delay = o_edge - i_edge;

                if (o_from == 0)
                    arc.SetRiseDelay(i_tran_index, o_cap_index, delay);
                else
                    arc.SetFallDelay(i_tran_index, o_cap_index, delay);
            }
        }
    }
}

void Algorithms::MeasureComboTransitions(Cell &cell)
{
    assert (cell.GetKind() == CellKind::COMBINATIONAL);

    Variables &vars = ctx_->GetVariables();

    double slew_upper_rise = vars.GetDoubleVariable("slew_upper_rise");
    double slew_upper_fall = vars.GetDoubleVariable("slew_upper_fall");
    double slew_lower_rise = vars.GetDoubleVariable("slew_lower_rise");
    double slew_lower_fall = vars.GetDoubleVariable("slew_lower_fall");

    Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
    Volt vdd_voltage = supply->GetVddVoltage();
    std::string vdd_name = supply->GetVddName();

    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {
            for (Simulation *sim : arc.GetSimulations()) {

                Waves w = sim->ReadWaves();

                int i_tran_index = sim->GetMetaDataAt(0);
                int o_cap_index = sim->GetMetaDataAt(1);
                int o_from = sim->GetMetaDataAt(3);

                double lo_th = (o_from == 0) ? slew_lower_rise : slew_lower_fall;
                double hi_th = (o_from == 0) ? slew_upper_rise : slew_upper_fall;

                lo_th *= vdd_voltage;
                hi_th *= vdd_voltage;

                NanoSecond lo_time = w.FindTransitionTime(o_pin.name_, o_from, lo_th);
                NanoSecond hi_time = w.FindTransitionTime(o_pin.name_, o_from, hi_th);

                if (o_from == 0)
                    arc.SetRiseTransition(i_tran_index, o_cap_index, hi_time - lo_time);
                else
                    arc.SetFallTransition(i_tran_index, o_cap_index, lo_time - hi_time);
            }
        }
    }
}

void Algorithms::MeasureComboPowers(Cell &cell)
{
    assert (cell.GetKind() == CellKind::COMBINATIONAL);

    Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
    Volt vdd_voltage = supply->GetVddVoltage();
    std::string vdd_name = supply->GetVddName();

    for (auto &o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto &arc : o_pin.GetArcs()) {
            for (Simulation *sim : arc.GetSimulations()) {

                Waves w = sim->ReadWaves();
                Pin *related_pin = arc.GetRelatedPin();

                int i_tran_index = sim->GetMetaDataAt(0);
                int o_cap_index = sim->GetMetaDataAt(1);
                int i_from = sim->GetMetaDataAt(2);
                int o_from = sim->GetMetaDataAt(3);

                std::vector<NanoWatt> pwr;
                std::vector<MicroAmp> i_vdd = w.GetCurrent(vdd_name);
                std::vector<MicroAmp> i_out = w.GetCurrent(o_pin.name_);

                NanoWatt lkg = i_vdd[0] * vdd_voltage * 1E3;

                assert (i_vdd.size() == i_out.size());

                for (size_t i = 0; i < i_vdd.size(); i++) {
                    pwr.push_back(((i_vdd[i] + i_out[i]) * vdd_voltage * 1E3) - lkg);
                }

                // Integrate total energy between the transitions between 1 and 99 percent
                // TODO: Figure out if this is the proper way!
                Volt th_start = (i_from == 0) ? vdd_voltage * 0.01 : vdd_voltage * 0.99;
                Volt th_end = (o_from == 0) ? vdd_voltage * 0.99 : vdd_voltage * 0.01;

                size_t pwr_start = w.FindTransitionIndex(related_pin->name_, i_from, th_start);
                size_t pwr_end = w.FindTransitionIndex(o_pin.name_, o_from, th_end);

                PicoJoule e = 0;
                NanoSecond step = sim->GetTimeStep();

                for (size_t i = pwr_start; i < pwr_end; i++) {
                    e += pwr[i] * step;
                }

                // ns * nW gives us "atto Joule" -> divide to get pJ
                e /= 1E6;

                if (o_from == 0) {
                    arc.SetFallPower(i_tran_index, o_cap_index, e);
                } else {
                    arc.SetRisePower(i_tran_index, o_cap_index, e);
                }
            }
        }
    }
}

// Now works for D-type latch or flop. R/S or J-K flop don't work!
void Algorithms::PrepareSeqAsyncFunctionSims(Cell &cell)
{
    size_t a_pin_cnt = 0;
    for ([[maybe_unused]] auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
        a_pin_cnt++;
    }

    if (a_pin_cnt == 0) {
        return;
    }

    assert(a_pin_cnt == 1 || a_pin_cnt == 2);

    size_t d_pin_cnt = 0;
    for ([[maybe_unused]] auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
        d_pin_cnt++;
    }

    // Support only single bit flip flops.
    assert(d_pin_cnt == 1);

    for (int64_t a_pin_vect = 0; a_pin_vect < (1 << a_pin_cnt); a_pin_vect++) {

        auto d_pin = cell.GetPins(PinDirection::IN, PinKind::DATA).front();

        for (int d = 0; d < 2; d++) {
            std::string sim_name = "SEQ_ASYNCFUNC";

            int64_t tmp = a_pin_vect;
            for (const auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                sim_name = sprintf("%s_%s%d", sim_name, a_pin.name_, tmp & 0x1);
                tmp >>= 1;
            }

            sim_name = sprintf("%s_%s%d", sim_name, d_pin.name_, d);

            // TODO: Wrap construction to some common tasks

            Simulation *sim = new Simulation(ctx_, sim_name, &cell, SimulationKind::TRAN);

            OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
            sim->SetTemp(op_cond.GetTemperature());
            sim->SetSupply(op_cond.GetSupply());

            for (const auto & netlist : ctx_->GetNetlists())
                sim->AddInclude(netlist);

            for (const auto & model : ctx_->GetModels())
                sim->AddModel(model);

            Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
            Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

            // Drive async pins
            tmp = a_pin_vect;
            for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                sim->AddStimuli((Pin*)&a_pin, Stimulus(((tmp & 0x1) == 1) ? log1_v : log0_v));
                tmp >>= 1;
            }

            // Drive data pins
            // TODO: This assumes having D-type sequential element.
            // For arbitrary one, we would need all combinations of inputs in
            // single simulation and recognize "clear candidate" and "set candidate"
            // when "no edge happened" based on output value!
            for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
                Volt v = (d == 1) ? log1_v : log0_v;
                sim->AddStimuli((Pin*)&i_pin, Stimulus(v));
            }

            // CK pulse
            for (auto & c_pin : cell.GetPins(PinKind::CLK)) {
                sim->AddStimuli((Pin*)&c_pin, Stimulus(log0_v, log1_v, 1.0, 0.01, 0.01, 1.0, 100.0, 1));
            }

            sim->PutMetaData(static_cast<int>(a_pin_vect));
            sim->PutMetaData(d);

            cell.AddSimulation(sim);
            ctx_->GetSimulationPool().PushSimulation(sim);
        }
    }
}

void Algorithms::MeasureSeqAsyncFunctions(Cell &cell)
{
    assert(cell.GetKind() == CellKind::SEQUENTIAL);

    struct a_pin_data {
        int64_t a_pin_vect;
        int  d;
        bool clr_candidate;
        bool set_candidate;
    };
    std::vector<a_pin_data> tbl;

    size_t n_set_candidates = 0;
    size_t n_clr_candidates = 0;

    for (auto & a_pin : cell.GetPins(PinKind::ASYNC))
        printf("%s ", a_pin.name_);

    printf("D SC CC Q\n");
    for (Simulation *sim : cell.GetSimulations()) {
        a_pin_data row = {};
        row.a_pin_vect = sim->GetMetaDataAt(0);
        row.d = sim->GetMetaDataAt(1);

        Waves w = sim->ReadWaves();
        auto q_pin = cell.GetPins(PinDirection::OUT, PinKind::DATA).front();
        Volt q_val = w.GetVoltage(q_pin.name_).back();

        if (row.d == 0 && ToLogic(q_val) == 1) {
            n_set_candidates++;
            row.set_candidate = true;
        } else if (row.d == 1 && ToLogic(q_val) == 0) {
            n_clr_candidates++;
            row.clr_candidate = true;
        }
        for (int i = 0; i < 2; i++) {
            printf("%b  ", (row.a_pin_vect >> i) & 0x1);
        }
        printf("%d  %d  %d  %f\n",
                row.d, row.set_candidate, row.clr_candidate, q_val);

        tbl.push_back(row);
    }

    assert( (n_set_candidates == 0 && n_clr_candidates == 1) ||
            (n_set_candidates == 1 && n_clr_candidates == 0) ||
            (n_set_candidates == 1 && n_clr_candidates == 2) ||
            (n_set_candidates == 2 && n_clr_candidates == 1));

    Expression *set_e = new Expression(ExpressionKind::CONSTANT, 0);
    Expression *clr_e = new Expression(ExpressionKind::CONSTANT, 0);

    for (auto & row : tbl) {
        if (!row.set_candidate && !row.clr_candidate) {
            continue;
        }

        assert (!(row.set_candidate && row.clr_candidate));

        size_t a_pin_vect = row.a_pin_vect;
        Expression *e = new Expression(ExpressionKind::CONSTANT, 1);

        for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
            Expression *e2 = new Expression(ExpressionKind::TERM, &a_pin);

            if ((a_pin_vect & 0x1) == 0) {
                e2 = new Expression(ExpressionKind::NOT, e2);
            }
            e = new Expression(ExpressionKind::AND, e, e2);
            a_pin_vect >>= 1;
        }

        if (row.set_candidate) {
            set_e = new Expression(ExpressionKind::OR, set_e, e);
        }

        if (row.clr_candidate) {
            clr_e = new Expression(ExpressionKind::OR, clr_e, e);
        }
    }

    clr_e->Simplify();
    set_e->Simplify();

    // Recognize case when we have only one async pin
    if (set_e->GetKind() == ExpressionKind::CONSTANT) {
        delete set_e;
        assert(clr_e->GetKind() == ExpressionKind::NOT ||
               clr_e->GetKind() == ExpressionKind::TERM);
        cell.GetSequential().SetClear(clr_e);
    }

    if (clr_e->GetKind() == ExpressionKind::CONSTANT) {
        delete clr_e;
        assert(set_e->GetKind() == ExpressionKind::NOT ||
               set_e->GetKind() == ExpressionKind::TERM);
        cell.GetSequential().SetPreset(set_e);
    }

    // Now we should have the async pin with priority containing only
    // TERM or NOT expression based on polarity!
    // TODO: This assumes that always one async pin will have priority
    // which might not be the case since liberty allows for other
    // kinds of behavior!
    assert ((set_e->GetKind() == ExpressionKind::TERM) ||
            (set_e->GetKind() == ExpressionKind::NOT)  ||
            (clr_e->GetKind() == ExpressionKind::TERM) ||
            (clr_e->GetKind() == ExpressionKind::NOT));

    Expression *dom_e;

    if (set_e->GetKind() == ExpressionKind::TERM || set_e->GetKind() == ExpressionKind::NOT) {
        dom_e = set_e;
    } else {
        dom_e = clr_e;
    }

    // The recessive async pin still contains the condition of inactive
    // dominant pin, otherwise it would not be recognized! We substitute
    // this out with inactive value!
    if (dom_e == set_e) {
        cell.GetSequential().SetAsyncPriority(AsyncPriority::PRESET);
        if (set_e->GetKind() == ExpressionKind::NOT) {
            Pin *set_pin = set_e->GetLhs()->GetPin();
            assert(set_pin != nullptr);
            clr_e->Substitute(set_pin, 1);
        } else {
            Pin *set_pin = set_e->GetPin();
            assert(set_pin != nullptr);
            clr_e->Substitute(set_pin, 0);
        }
        clr_e->Simplify();
    } else {
        cell.GetSequential().SetAsyncPriority(AsyncPriority::CLEAR);
        if (clr_e->GetKind() == ExpressionKind::NOT) {
            Pin *clr_pin = clr_e->GetLhs()->GetPin();
            assert (clr_pin != nullptr);
            set_e->Substitute(clr_pin, 1);
        } else {
            Pin *clr_pin = clr_e->GetPin();
            assert (clr_pin != nullptr);
            set_e->Substitute(clr_pin, 0);
        }
        set_e->Simplify();
    }

    if (clr_e->GetKind() == ExpressionKind::NOT) {
        clr_e->GetLhs()->GetPin()->SetPolarity(0);
    } else {
        clr_e->GetPin()->SetPolarity(1);
    }

    if (set_e->GetKind() == ExpressionKind::NOT) {
        set_e->GetLhs()->GetPin()->SetPolarity(0);
    } else {
        set_e->GetPin()->SetPolarity(1);
    }

    cell.GetSequential().SetPreset(set_e);
    cell.GetSequential().SetClear(clr_e);
}

void Algorithms::PrepareSeqCellKindSims(Cell &cell)
{
    for (int clock_val = 0; clock_val < 2; clock_val++) {
        std::string sim_name = "SEQ_EDGEVSLVL";

        Pin &c_pin = cell.GetPins(PinKind::CLK).front();
        sim_name = sprintf("%s_%s%d", sim_name, c_pin.name_, clock_val);

        // TODO: Wrap construction to some common tasks

        Simulation *sim = new Simulation(ctx_, sim_name, &cell, SimulationKind::TRAN);

        OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
        sim->SetTemp(op_cond.GetTemperature());
        sim->SetSupply(op_cond.GetSupply());

        for (const auto & netlist : ctx_->GetNetlists())
            sim->AddInclude(netlist);

        for (const auto & model : ctx_->GetModels())
            sim->AddModel(model);

        Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
        Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

        // Tie async pins to inactive value
        for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
            sim->AddStimuli(&a_pin, Stimulus(((a_pin.GetPolarity()) == 1) ? log0_v : log1_v));
        }

        // Drive data pins to all combinations of inputs
        NanoSecond pw = 1.0;
        for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
            sim->AddStimuli(&i_pin, Stimulus(log0_v, log1_v, pw, 0.01, 0.01, pw, pw * 2, 32));
            pw *= 2;
        }

        // Scale the simulation so that we manage to try all input values!
        sim->SetDuration(pw);

        // Tie clock pin based on simulation type being performed
        for (auto & c_pin : cell.GetPins(PinKind::CLK)) {
            sim->AddStimuli(&c_pin, Stimulus((clock_val == 0) ? log0_v : log1_v));
        }

        sim->PutMetaData(clock_val);

        cell.AddSimulation(sim);
        ctx_->GetSimulationPool().PushSimulation(sim);
    }
}

void Algorithms::MeasureSeqCellKind(Cell &cell)
{
    bool is_latch = false;

    for (Simulation *sim : cell.GetSimulations()) {
        // TODO: Find better way of filtering sims from previous ones ?
        if (!sim->name_.starts_with("SEQ_EDGEVSLVL")) {
            continue;
        }

        Waves w = sim->ReadWaves();

        bool has_logic_0 = false;
        bool has_logic_1 = false;

        for (auto & pin : cell.GetPins(PinDirection::OUT)) {
            for (Volt v : w.GetVoltage(pin.name_)) {
                if (ToLogic(v) == 1) {
                    has_logic_1 = true;
                } else if (ToLogic(v) == 0) {
                    has_logic_0 = true;
                }
            }
        }

        if (has_logic_0 && has_logic_1) {
            is_latch = true;
            int active_clock = sim->GetMetaDataAt(0);
            cell.GetSequential().SetEnablePolarity(active_clock);
        }
    }

    if (is_latch) {
        cell.GetSequential().SetKind(SequentialKind::LATCH);
    } else {
        cell.GetSequential().SetKind(SequentialKind::FLIP_FLOP);
    }
}

void Algorithms::PrepareFFClockPolaritySims(Cell &cell)
{
    size_t i_pin_cnt = 0;
    // TODO: Wrap calculation of number of pins!
    for ([[maybe_unused]] auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
        i_pin_cnt++;
    }

    for (int clock_polarity = 0; clock_polarity < 2; clock_polarity++) {

        size_t test_vect_cnt = 1 << i_pin_cnt;
        for (size_t i_pin_vect = 0 ; i_pin_vect < test_vect_cnt; i_pin_vect++) {

            std::string sim_name = sprintf("FF_CKPOL_%s%d%d", cell.GetPins(PinKind::CLK).front().name_,
                                            clock_polarity, 1 - clock_polarity);

            size_t i = 0;
            for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, (i_pin_vect >> i) & 0x1);
                i++;
            }

            // TODO: Wrap construction to some common tasks

            Simulation *sim = new Simulation(ctx_, sim_name, &cell, SimulationKind::TRAN);

            OpCond& op_cond = ctx_->GetLibrary().GetOpCond();
            sim->SetTemp(op_cond.GetTemperature());
            sim->SetSupply(op_cond.GetSupply());

            for (const auto & netlist : ctx_->GetNetlists())
                sim->AddInclude(netlist);

            for (const auto & model : ctx_->GetModels())
                sim->AddModel(model);

            Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
            Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

            sim->SetDuration(2);

            // Tie async pins to inactive value
            for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                sim->AddStimuli(&a_pin, Stimulus(((a_pin.GetPolarity()) == 1) ? log0_v : log1_v));
            }

            // Drive input data pins
            i = 0;
            for (auto & i_pin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
                int i_pin_val = (i_pin_vect >> i) & 0x1;
                sim->AddStimuli(&i_pin, (i_pin_val == 1) ? log1_v : log0_v);
                i++;
            }

            // Generate clock edge
            for (auto & c_pin : cell.GetPins(PinKind::CLK)) {
                sim->AddStimuli(&c_pin, Stimulus((clock_polarity == 0) ? log0_v : log1_v,
                                                 (clock_polarity == 0) ? log1_v : log0_v,
                                                1.0, 0.01, 0.01, 2, 2, 1));
            }

            sim->PutMetaData(clock_polarity);

            cell.AddSimulation(sim);
            ctx_->GetSimulationPool().PushSimulation(sim);
        }
    }
}

void Algorithms::MeasureFFClockPolarity(Cell &cell)
{
    assert(cell.GetSequential().GetKind() == SequentialKind::FLIP_FLOP);

    bool out_change_posedge = false;
    bool out_change_negedge = false;

    for (Simulation *sim : cell.GetSimulations()) {
        // TODO: Find better way of filtering sims from previous ones ?
        if (!sim->name_.starts_with("FF_CKPOL")) {
            continue;
        }

        Waves w = sim->ReadWaves();

        for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
            auto &out_v = w.GetVoltage(o_pin.name_);

            if (!COMPARE_FLOATS(out_v.front(), out_v.back())) {
                int clock_polarity = sim->GetMetaDataAt(0);
                if (clock_polarity == 0) {
                    out_change_posedge = true;
                } else {
                    out_change_negedge = true;
                }
            }
        }
    }

    // TODO: Throw user visible error when changes occur on both edges
    assert(out_change_negedge || out_change_posedge);
    assert(!(out_change_negedge && out_change_posedge));

    if (out_change_posedge) {
        cell.GetSequential().SetClockPolarity(EdgeKind::RISING);
    } else {
        cell.GetSequential().SetClockPolarity(EdgeKind::FALLING);
    }
}

void Algorithms::PrepareSeqClockDelaySims(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {

        auto *templ = cell.GetDelayTemplate();

        size_t i_tran_index = 0;
        for (NanoSecond i_tran : templ->GetIndex1()) {

            size_t o_cap_index = 0;
            for (PicoFarad o_cap : templ->GetIndex2()) {

                // TODO: This assumes D-style flip-flop or latch.
                for (int d_val = 0; d_val < 2; d_val++) {

                    // TODO: This is hard-coded for Q with the same polarity!
                    // We should extract logic function of each Q and compute according to it!
                    // Also, this is missing values of async pins.
                    o_pin.AddArc(Arc(&o_pin, templ, d_val, 1 - d_val, d_val, 1 - d_val));
                    size_t arc_index = o_pin.GetArcs().size() - 1;

                    std::string sim_name = "SEQ_DLY";

                    auto & c_pin = cell.GetPins(PinKind::CLK).front();
                    sim_name = sprintf("%s_%s", sim_name, c_pin.name_);

                    auto & d_pin = cell.GetPins(PinDirection::IN, PinKind::DATA).front();
                    sim_name = sprintf("%s_%s%d", sim_name, d_pin.name_, d_val);

                    sim_name = sprintf("%s_TRAN_%f_CAP_%f", sim_name, i_tran, o_cap);

                    // TODO: Wrap construction to some common tasks

                    Simulation *sim = new Simulation(ctx_, sim_name, &cell, SimulationKind::TRAN);

                    OpCond & op_cond = ctx_->GetLibrary().GetOpCond();
                    sim->SetTemp(op_cond.GetTemperature());
                    sim->SetSupply(op_cond.GetSupply());

                    for (const auto & netlist : ctx_->GetNetlists())
                        sim->AddInclude(netlist);

                    for (const auto & model : ctx_->GetModels())
                        sim->AddModel(model);

                    Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
                    Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

                    sim->SetDuration(25);

                    // Tie async pins to inactive value
                    for (auto & a_pin : cell.GetPins(PinKind::ASYNC)) {
                        sim->AddStimuli(&a_pin, Stimulus(((a_pin.GetPolarity()) == 1) ? log0_v : log1_v));
                    }

                    // Drive D-pin
                    sim->AddStimuli(&d_pin, Stimulus((d_val == 1) ? log0_v : log1_v,
                                                     (d_val == 1) ? log1_v : log0_v,
                                                     0.1, 0.01, 0.01, 10.0, 100.0, 1));

                    // Drive clock
                    // TODO: Support latches here!
                    EdgeKind c_polarity = cell.GetSequential().GetClockPolarity();
                    sim->AddStimuli(&c_pin, Stimulus(
                                        (c_polarity == EdgeKind::RISING) ? log0_v : log1_v,
                                        (c_polarity == EdgeKind::RISING) ? log1_v : log0_v,
                                         5.0, i_tran, i_tran, 5.0, 10.0, 2));

                    // Add load
                    sim->AddLoad(&o_pin, o_cap);

                    sim->PutMetaData(static_cast<int>(i_tran_index));
                    sim->PutMetaData(static_cast<int>(o_cap_index));

                    sim->PutMetaData(d_val);

                    o_pin.GetArcs()[arc_index].AddSimulation(sim);
                    ctx_->GetSimulationPool().PushSimulation(sim);
                }
                o_cap_index++;
            }
            i_tran_index++;
        }
    }
}

void Algorithms::MeasureSeqClockDelay(Cell &cell)
{
    for (auto & o_pin : cell.GetPins(PinDirection::OUT)) {
        for (auto & arc : o_pin.GetArcs()) {
            for (Simulation *sim : arc.GetSimulations()) {
                Waves w = sim->ReadWaves();

                int i_tran_index = sim->GetMetaDataAt(0);
                int o_cap_index = sim->GetMetaDataAt(1);

                // TODO: Search for delay between D and Q on the second clock edge!
            }
        }
    }
}

void Algorithms::MeasureSeqClockTransition(Cell &cell)
{

}

void Algorithms::MeasureSeqClockPowers(Cell &cell)
{

}


void Algorithms::CharacterizeLibrary()
{
    SimulationPool &sp = ctx_->GetSimulationPool();

    // TODO: Set by TCL variable
    sp.SetNumThreads(14);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // First stage
    ///////////////////////////////////////////////////////////////////////////////////////////////

    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::COMBINATIONAL) {
            info("%s - Preparing Combinatorial Logic table simulations", cell.second.GetName());
            PrepareComboLogicTableAndLeakageSims(cell.second);
        } else {
            info("%s - Preparing Asynchronous pins simulations", cell.second.GetName());
            PrepareSeqAsyncFunctionSims(cell.second);
        }
    }

    info("Running first simulation stage");
    sp.RunSimulations();

    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::COMBINATIONAL) {
            info("%s - Measuring logic table", cell.second.GetName());
            MeasureComboLogicTables(cell.second);
            info("%s - Measuring leakage", cell.second.GetName());
            MeasureComboLeakage(cell.second);
            info("%s - Calculating logic function", cell.second.GetName());
            CalculateComboLogicFunctions(cell.second);
        } else {
            info("%s - Measuring async pin functions", cell.second.GetName());
            MeasureSeqAsyncFunctions(cell.second);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Second Simulation stage
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Prepare
    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::COMBINATIONAL) {
            info("%s - Preparing elay and power simulations", cell.second.GetName());
            PrepareComboDelayAndPowerSims(cell.second);
        } else {
            info("%s - Preparing sequential cell kind detection simulations", cell.second.GetName());
            PrepareSeqCellKindSims(cell.second);
        }
    }

    info("Running second simulation stage");
    sp.RunSimulations();

    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::COMBINATIONAL) {
            info("%s - Measuring output delays", cell.second.GetName());
            MeasureComboDelays(cell.second);

            info("%s - Measuring output transitions", cell.second.GetName());
            MeasureComboTransitions(cell.second);

            info("%s - Measuring internal power", cell.second.GetName());
            MeasureComboPowers(cell.second);
        } else {
            info("%s - Measuring if cell is level sensitive or edge sensitive",
                cell.second.GetName());
            MeasureSeqCellKind(cell.second);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Third Simulation stage
    ///////////////////////////////////////////////////////////////////////////////////////////////
    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::SEQUENTIAL) {
            if (cell.second.GetSequential().GetKind() == SequentialKind::FLIP_FLOP) {
                info("%s - Preparing flip-flop clock polarity detection simulations",
                     cell.second.GetName());
                PrepareFFClockPolaritySims(cell.second);
            }
        }
    }

    info("Running third simulation stage");
    sp.RunSimulations();

    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::SEQUENTIAL) {
            if (cell.second.GetSequential().GetKind() == SequentialKind::FLIP_FLOP) {
                info("%s - Measuring clock polarity", cell.second.GetName());
                MeasureFFClockPolarity(cell.second);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Fourth simulation stage
    ///////////////////////////////////////////////////////////////////////////////////////////////
    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::SEQUENTIAL) {
            info("%s - Preparing clock to output delay simulations", cell.second.GetName());
            PrepareSeqClockDelaySims(cell.second);
        }
    }

    info("Running fourth simulation stage");
    sp.RunSimulations();

}

}
