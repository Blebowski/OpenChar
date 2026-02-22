

#include <bit>

#include "Algorithms.h"
#include "Context.h"
#include "Arc.h"
#include "Library.h"
#include "Simulation.h"
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
                    //printf("PWR is: %.10f nW, Step is: %f NS\n", pwr[i], step);
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

void Algorithms::PrepareSeqAsyncFunctionSims(Cell &cell)
{
    auto apins = cell.GetPins(PinKind::ASYNC);

    size_t apins_cnt = 0;
    for (auto apin : apins) {
        apins_cnt++;
    }

    if (apins_cnt == 0) {
        return;
    }

    for (int64_t apin_vect = 0; apin_vect < (1 << apins_cnt); apin_vect++) {

        std::string sim_name = "SEQ_ASYNCFUNC";

        int64_t tmp = apin_vect;
        for (const auto & apin : apins) {
            sim_name = sprintf("%s_%s%d", sim_name, apin.name_, tmp & 0x1);
            tmp >>= 1;
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

        // Drive async pins
        tmp = apin_vect;
        for (auto & apin : apins) {
            sim->AddStimuli((Pin*)&apin, Stimulus(((tmp & 0x1) == 1) ? log1_v : log0_v));
            tmp >>= 1;
        }

        // Drive data pins
        for (auto & ipin : cell.GetPins(PinDirection::IN, PinKind::DATA)) {
            sim->AddStimuli((Pin*)&ipin, Stimulus(0));
        }

        // CK pulse
        for (auto & cpin : cell.GetPins(PinKind::CLK)) {
            sim->AddStimuli((Pin*)&cpin, Stimulus(log0_v, log1_v, 1, 0.1, 0.1, 1, 100, 1));
        }

        ctx_->GetSimulationPool().PushSimulation(sim);
    }
}

Expression* Algorithms::SumOfProducts(Cell& cell, Pin& opin)
{
    Expression *e = new Expression(ExpressionKind::CONSTANT, 0);

    for (auto &lt_entry : opin.GetLogicTable()) {
        int output = lt_entry.second;

        Expression* row_e = new Expression(ExpressionKind::CONSTANT, 1);

        if (output == 0)
            continue;

        int64_t inputs = lt_entry.first;

        int i = 0;
        for (auto &ipin : cell.GetPins(PinDirection::IN)) {
            int val = (inputs >> i) & 0x1;

            Expression *e_rhs = new Expression(ExpressionKind::TERM, &ipin);
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

Expression* Algorithms::ProductOfSums(Cell& cell, Pin& opin)
{
    // TODO: Cross-check this is correct!
    Expression *e = new Expression(ExpressionKind::CONSTANT, 1);

    for (const auto &lt_entry : opin.GetLogicTable()) {
        int output = lt_entry.second;

        Expression* row_e = new Expression(ExpressionKind::CONSTANT, 0);

        if (output == 1)
            continue;

        int64_t inputs = lt_entry.first;

        int i = 0;
        for (auto &ipin : cell.GetPins(PinDirection::IN)) {
            int val = (inputs >> i) & 0x1;

            Expression *e_rhs = new Expression(ExpressionKind::TERM, &ipin);
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

Expression* Algorithms::RecognizeXor(Cell& cell, Pin& opin)
{
    size_t n_inputs = cell.GetPinsCount(PinDirection::IN);

    // Checks all rows are XOR of inputs
    for (const auto & row : opin.GetLogicTable()) {
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

    for (auto & ipin : cell.GetPins(PinDirection::IN)) {
        Expression *term = new Expression(ExpressionKind::TERM, &ipin);
        e = new Expression(ExpressionKind::XOR, e, term);
    }

    return e;
}

void Algorithms::CalculateComboLogicFunctions(Cell &cell)
{
    for (auto & opin : cell.GetPins(PinDirection::OUT)) {
        auto &log_table = opin.GetLogicTable();
        assert (log_table.size() > 0);

        Expression *e = RecognizeXor(cell, opin);

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
                e = ProductOfSums(cell, opin);
            } else {
                e = SumOfProducts(cell, opin);
            }
        }

        e->Simplify();
        opin.SetLogicFunction(e);
        opin.PrintLogicFunction();
    }
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
            PrepareComboLogicTableAndLeakageSims(cell.second);
        } else {
            PrepareSeqAsyncFunctionSims(cell.second);
        }
    }

    sp.RunSimulations();

    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::COMBINATIONAL) {
            MeasureComboLogicTables(cell.second);
            MeasureComboLeakage(cell.second);
            CalculateComboLogicFunctions(cell.second);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Second Simulation stage
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Prepare
    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        PrepareComboDelayAndPowerSims(cell.second);
    }

    sp.RunSimulations();

    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::COMBINATIONAL) {
            info("%s - Measuring output delays", cell.second.GetName());
            MeasureComboDelays(cell.second);

            info("%s - Measuring output transitions", cell.second.GetName());
            MeasureComboTransitions(cell.second);

            info("%s - Measuring internal power", cell.second.GetName());
            MeasureComboPowers(cell.second);
        }
    }

}

}
