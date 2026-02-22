

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
    Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
    Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

    auto o_pins = cell.GetPins(PinDirection::OUT);

    bool first_opin = true;
    for (auto & o_pin : o_pins) {
        auto i_pins = cell.GetPins(PinDirection::IN);
        size_t i_pins_len = cell.GetPinsCount(PinDirection::IN);

        size_t n_sims = 1;
        for (size_t i = 0; i < i_pins_len; i++)
            n_sims *= 2;

        for (int64_t ipin_vect = 0; ipin_vect < static_cast<int64_t>(n_sims); ipin_vect++) {

            std::string sim_name = "COMBO_LOGTBLLKG";
            size_t input = ipin_vect;

            for (const auto & i_pin : i_pins) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, input & 0x1);
                input >>= 1;
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
            for (auto & i_pin : i_pins) {
                size_t i_pin_val = ((ipin_vect >> i_pin_index) & 0x1);
                sim->AddStimuli((Pin*)&i_pin, Stimulus((i_pin_val == 1) ? log1_v : log0_v));

                i_pin_index++;
            }

            auto post_sim_cb = [this, sim, &cell, &o_pin, &op_cond, ipin_vect, first_opin] () {
                Waves w = sim->ReadWaves();
                w.Print();

                // Output value upon logic inputs
                o_pin.AddLogicTableEntry(ipin_vect, ToLogic(w.GetVoltage(o_pin.name_)[0]));

                // Leakage power upon this input combination
                if (first_opin) {
                    Supply *s = op_cond.GetSupply();

                    NanoWatt lkg = w.GetCurrent(s->GetVddName())[0] * s->GetVddVoltage() * 1E3;

                    Expression *e = new Expression(ExpressionKind::CONSTANT, 1);
                    size_t v = ipin_vect;
                    auto i_pins = cell.GetPins(PinDirection::IN);
                    for (auto & i_pin : i_pins) {
                        Expression *tmp = new Expression(ExpressionKind::TERM, &(i_pin));
                        if ((v & 0x1) == 0) {
                            tmp = new Expression(ExpressionKind::NOT, tmp);
                        }
                        e = new Expression(ExpressionKind::AND, e, tmp);
                        v >>= 1;
                    }
                    e->Simplify();
                    cell.AddLeakageTableEntry(e, lkg);
                }

                // TODO: check for errors ?
                return 0;
            };

            sim->SetPostSimCb(post_sim_cb);

            ctx_->GetSimulationPool().PushSimulation(sim);
        }
        first_opin = false;
    }
}

int Algorithms::PrepareComboArcSims(Pin *opin, int64_t in_a, int64_t in_b, int out_a, int out_b)
{
    Cell *cell = opin->cell_;
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
    Template *templ = cell->GetDelayTemplate();

    opin->AddArc(Arc(opin, templ, in_a, in_b, out_a, out_b));
    size_t arc_index = opin->GetArcs().size() - 1;

    for (size_t i = 0; i < 2; i++) {
        int64_t in_from     = (i == 0) ? in_a  : in_b;
        int64_t in_to       = (i == 0) ? in_b  : in_a;
        int     out_from    = (i == 0) ? out_a : out_b;

        std::string prefix = "COMBO_DLYPWR";
        size_t j = 0;
        for (const auto & i_pin : cell->GetPins(PinDirection::IN)) {
            prefix = sprintf("%s_%s%d%d", prefix, i_pin.name_, GetBit(in_from, j), GetBit(in_to, j));
            j++;
        }

        int i_tran = 0;
        for (const NanoSecond in_tran : templ->GetIndex1()) {

            int i_cap = 0;
            for (const PicoFarad out_cap : templ->GetIndex2()) {

                std::string sim_name = sprintf("%s_TRAN_%f_CAP_%f", prefix, in_tran, out_cap);

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

                int i = 0;
                Pin *tran_pin = nullptr;
                int tran_from = 0;

                for (auto & i_pin : cell->GetPins(PinDirection::IN)) {

                    int in_from_bit = GetBit(in_from, i);
                    int in_to_bit = GetBit(in_to, i);

                    if (in_from_bit == in_to_bit) {
                        sim->AddStimuli((Pin*)&i_pin,
                                        Stimulus((in_from_bit == 1) ? log1_v : log0_v));
                    } else {

                        assert((in_from_bit == 0 && in_to_bit == 1) |
                               (in_from_bit == 1 && in_to_bit == 0));

                        NanoSecond in_tran_cor = in_tran;
                        Variables &vars = ctx_->GetVariables();

                        // Assumes linear ramp
                        if (in_from_bit == 0) {
                            double slew_lower_rise = vars.GetDoubleVariable("slew_lower_rise");
                            double slew_upper_rise = vars.GetDoubleVariable("slew_upper_rise");
                            in_tran_cor *= (1 + slew_lower_rise + (1 - slew_upper_rise));
                        } else {
                            double slew_lower_fall = vars.GetDoubleVariable("slew_lower_fall");
                            double slew_upper_fall = vars.GetDoubleVariable("slew_upper_fall");
                            in_tran_cor *= (1 + slew_lower_fall + (1 - slew_upper_fall));
                        }

                        // TODO: Refine
                        Stimulus edge (
                            (in_from_bit == 1) ? log1_v : log0_v,
                            (in_to_bit == 1)   ? log1_v : log0_v,
                            1,
                            in_tran_cor,
                            in_tran_cor,
                            100,
                            100,
                            1
                        );
                        sim->AddStimuli((Pin*)&i_pin, std::move(edge));
                        tran_pin = &i_pin;
                        tran_from = in_from_bit;
                    }

                    i++;
                }
                sim->AddLoad(opin, out_cap);

                auto post_sim_cb = [this, sim, tran_pin, opin, i_tran,
                                    tran_from, out_from, arc_index] () {

                    Waves w = sim->ReadWaves();
                    Arc& arc = opin->GetArcs()[arc_index];

                    assert(tran_pin != nullptr);
                    Variables &vars = ctx_->GetVariables();
                    Volt vdd = ctx_->GetLibrary().GetOpCond().GetSupply()->GetVddVoltage();
                    std::string vdd_name = ctx_->GetLibrary().GetOpCond().GetSupply()->GetVddName();

                    // Calculate output delay
                    double in_th = (tran_from == 0) ? vars.GetDoubleVariable("delay_in_rise") :
                                                      vars.GetDoubleVariable("delay_in_fall");
                    double out_th = (out_from == 0) ? vars.GetDoubleVariable("delay_out_rise") :
                                                      vars.GetDoubleVariable("delay_out_fall");

                    in_th *= vdd;
                    out_th *= vdd;
                    NanoSecond in_edge  = w.FindTransitionTime(tran_pin->name_, tran_from, in_th);
                    NanoSecond out_edge = w.FindTransitionTime(opin->name_, out_from, out_th);

                    if (out_from == 0)
                        arc.AddRiseDelay(i_tran, out_edge - in_edge);
                    else
                        arc.AddFallDelay(i_tran, out_edge - in_edge);

                    // Calculate output transition
                    double upp_th = (out_from == 0) ? vars.GetDoubleVariable("slew_upper_rise") :
                                                      vars.GetDoubleVariable("slew_upper_fall");
                    double low_th = (out_from == 0) ? vars.GetDoubleVariable("slew_lower_rise") :
                                                      vars.GetDoubleVariable("slew_lower_fall");

                    upp_th *= vdd;
                    low_th *= vdd;

                    NanoSecond low  = w.FindTransitionTime(opin->name_, out_from, low_th);
                    NanoSecond high = w.FindTransitionTime(opin->name_, out_from, upp_th);

                    if (out_from == 0)
                        arc.AddRiseTransition(i_tran, (out_from == 0) ? high - low :
                                                                               low - high);
                    else
                        arc.AddFallTransition(i_tran, (out_from == 0) ? high - low :
                                                                               low - high);

                    // Calculate power
                    std::vector<NanoWatt> pwr;
                    std::vector<MicroAmp> i_vdd = w.GetCurrent(vdd_name);
                    std::vector<MicroAmp> i_out = w.GetCurrent(opin->name_);

                    NanoWatt lkg = i_vdd[0] * vdd * 1E3;

                    assert (i_vdd.size() == i_out.size());

                    for (size_t i = 0; i < i_vdd.size(); i++) {
                        pwr.push_back(((i_vdd[i] + i_out[i]) * vdd * 1E3) - lkg);
                    }

                    // Integrate total energy between the transitions between 1 and 99 percent
                    // TODO: Figure out if this is the proper way!
                    Volt th_start = (tran_from == 0) ? vdd * 0.01 : vdd * 0.99;
                    Volt th_end = (out_from == 0) ? vdd * 0.99 : vdd * 0.01;

                    size_t pwr_start = w.FindTransitionIndex(tran_pin->name_, tran_from, th_start);
                    size_t pwr_end = w.FindTransitionIndex(opin->name_, out_from, th_end);

                    PicoJoule e = 0;
                    NanoSecond step = sim->GetTimeStep();

                    for (size_t i = pwr_start; i < pwr_end; i++) {
                        //printf("PWR is: %.10f nW, Step is: %f NS\n", pwr[i], step);
                        e += pwr[i] * step;
                    }

                    // ns * nW gives us "atto Joule" -> divide to get pJ
                    e /= 1E6;

                    if (out_from == 0) {
                        arc.AddFallPower(i_tran, e);
                    } else {
                        arc.AddRisePower(i_tran, e);
                    }

                    // TODO: Return some error code
                    return 0;
                };

                sim->SetPostSimCb(post_sim_cb);
                ctx_->GetSimulationPool().PushSimulation(sim);

                i_cap++;
            }
            i_tran++;
        }
    }

    return 0;
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
            PrepareComboArcSims(&o_pin, tv.in_i, tv.in_j, tv.out_i, tv.out_j);
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

        auto post_sim_cb = [](){

            // TODO: Recognize polarity and functionality of the async pins!
            return 0;
        };

        sim->SetPostSimCb(post_sim_cb);
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

void Algorithms::CalculateLogicFunctions(Cell &cell)
{
    for (auto & opin : cell.GetPins(PinDirection::OUT)) {
        auto &log_table = opin.GetLogicTable();
        assert(log_table.size() > 0);

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

    // Prepare
    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        if (cell.second.GetKind() == CellKind::COMBINATIONAL) {
            PrepareComboLogicTableAndLeakageSims(cell.second);
        } else {
            PrepareSeqAsyncFunctionSims(cell.second);
        }
    }

    // Simulate and post-process
    sp.StartSimulations();
    sp.FinishAndProcessSimulations();

    exit(1);

    // Manual calculations based on result
    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        CalculateLogicFunctions(cell.second);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Second Simulation stage
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // Prepare
    for (auto & cell : ctx_->GetLibrary().GetCells()) {
        PrepareComboDelayAndPowerSims(cell.second);
    }

    // Simulate and post-process
    sp.StartSimulations();
    sp.FinishAndProcessSimulations();
}

}
