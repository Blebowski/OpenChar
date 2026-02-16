

#include <bit>

#include "Algorithms.h"
#include "Context.h"
#include "DelayTable.h"
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

NanoWatt Algorithms::ComputePower(MicroAmp i, Volt v)
{
    return i * v * 1E3;
}

void Algorithms::MeasureLogicTableAndLeakage(Cell &cell)
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

        for (size_t ipin_vect = 0; ipin_vect < n_sims; ipin_vect++) {

            std::string sim_name = "LOG_TBL_LKG";
            size_t input = ipin_vect;

            for (const auto & i_pin : i_pins) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, input & 0x1);
                input >>= 1;
            }

            Simulation sim {ctx_, sim_name, &cell, SimulationKind::DC};

            sim.SetTemp(op_cond.GetTemperature());
            sim.SetSupply(op_cond.GetSupply());

            for (const auto & netlist : ctx_->GetNetlists())
                sim.AddInclude(netlist);

            for (const auto & model : ctx_->GetModels())
                sim.AddModel(model);

            int i_pin_index = 0;
            for (auto & i_pin : i_pins) {
                size_t i_pin_val = ((ipin_vect >> i_pin_index) & 0x1);
                sim.AddStimuli((Pin*)&i_pin, Stimulus((i_pin_val == 1) ? log1_v : log0_v));

                i_pin_index++;
            }

            // TODO: Propagate error
            sim.Simulate();

            Waves w = sim.ReadWaves();
            w.Print();

            // Output value upon logic inputs
            o_pin.AddLogicTableEntry(ipin_vect, ToLogic(w.GetVoltage(o_pin.name_)[0]));

            // Leakage power upon this input combination
            if (first_opin) {
                Supply *s = op_cond.GetSupply();
                NanoWatt lkg = Algorithms::ComputePower(
                                w.GetCurrent(s->GetVddName())[0], s->GetVddVoltage());

                Expression *e = new Expression(ExpressionKind::CONSTANT, 1);
                int v = ipin_vect;
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
        }
        first_opin = false;
    }

    for (auto & o_pin : o_pins) {
        o_pin.PrintLogicTable();
    }
}

NanoSecond Algorithms::FindVoltage(Waves &w, Pin *pin, int from, Volt v)
{
    const std::vector<Volt>& d = w.GetVoltage(pin->name_);
    size_t len = d.size();

    // TODO: Cross-check first and last data match the "from" and "to".

    Volt th = v * ctx_->GetLibrary().GetOpCond().GetSupply()->GetVddVoltage();

    size_t index = len - 1;
    size_t step = len / 2;

    while (step > 0) {
        Volt v = d[index];

        if (v > th) {
            if (from == 0) {
                index -= step;
            } else {
                index += step;
            }
        } else {
            if (from == 0) {
                index += step;
            } else {
                index -= step;
            }
        }

        step /= 2;
    }

    return w.GetTimeAtIndex(index);
}

int Algorithms::MeasureOneStateDelay(Pin *opin, int64_t in_from, int64_t in_to,
                                     int out_from, int out_to)
{
    Cell *cell = opin->cell_;
    auto i_pins = cell->GetPins(PinDirection::IN);
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();

    std::string prefix = "DLY";
    size_t i = 0;
    for (const auto & i_pin : i_pins) {
        prefix = sprintf("%s_%s%d%d", prefix, i_pin.name_, GetBit(in_from, i), GetBit(in_to, i));
        i++;
    }

    Template *templ = cell->GetDelayTemplate();
    DelayTable delay_table(opin, templ, in_from, in_to, out_from, out_to);

    int i_tran = 0;
    for (const NanoSecond in_tran : templ->index_1_) {

        int i_cap = 0;
        for (const PicoFarad out_cap : templ->index_2_) {

            std::string sim_name = sprintf("%s_TRAN_%f_CAP_%f", prefix, in_tran, out_cap);

            Simulation sim {ctx_, sim_name, cell, SimulationKind::TRAN};
            sim.SetTemp(op_cond.GetTemperature());
            sim.SetSupply(op_cond.GetSupply());

            for (const auto & netlist : ctx_->GetNetlists())
                sim.AddInclude(netlist);

            for (const auto & model : ctx_->GetModels())
                sim.AddModel(model);

            Volt log0_v = op_cond.GetSupply()->GetGndVoltage();
            Volt log1_v = op_cond.GetSupply()->GetVddVoltage();

            int i = 0;
            Pin *tran_pin = nullptr;
            int tran_from = 0;

            for (auto & i_pin : i_pins) {

                int in_from_bit = GetBit(in_from, i);
                int in_to_bit = GetBit(in_to, i);

                if (in_from_bit == in_to_bit) {
                    sim.AddStimuli((Pin*)&i_pin, Stimulus((in_from_bit == 1) ? log1_v : log0_v));
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
                          (in_to_bit == 1) ? log1_v : log0_v,
                         1,
                         in_tran_cor,
                         in_tran_cor,
                         100,
                         100,
                         1
                    );
                    sim.AddStimuli((Pin*)&i_pin, std::move(edge));
                    tran_pin = &i_pin;
                    tran_from = in_from_bit;
                }

                i++;
            }

            sim.AddLoad(opin, out_cap);

            // TODO: Propagate error
            sim.Simulate();
            Waves w = sim.ReadWaves();

            assert(tran_pin != nullptr);
            Variables &vars = ctx_->GetVariables();

            // Calculate output delay

            double in_th = (tran_from == 0) ? vars.GetDoubleVariable("delay_in_rise") :
                                              vars.GetDoubleVariable("delay_in_fall");
            double out_th = (out_from == 0) ? vars.GetDoubleVariable("delay_out_rise") :
                                              vars.GetDoubleVariable("delay_out_fall");

            NanoSecond in_edge  = FindVoltage(w, tran_pin, tran_from, in_th);
            NanoSecond out_edge = FindVoltage(w, opin, out_from, out_th);
            delay_table.AddDelay(i_tran, out_edge - in_edge);

            // Calculate output transition
            double upp_th = (out_from == 0) ? vars.GetDoubleVariable("slew_upper_rise") :
                                              vars.GetDoubleVariable("slew_upper_fall");
            double low_th = (out_from == 0) ? vars.GetDoubleVariable("slew_lower_rise") :
                                              vars.GetDoubleVariable("slew_lower_fall");

            NanoSecond low  = FindVoltage(w, tran_pin, tran_from, low_th);
            NanoSecond high = FindVoltage(w, tran_pin, tran_from, upp_th);

            delay_table.AddTransition(i_tran, (out_from == 0) ? high - low : low - high);

            i_cap++;
        }

        i_tran++;
    }

    delay_table.Print();
    opin->AddDelayTable(delay_table);

    return 0;
}

void Algorithms::MeasureComboDelay(Cell &cell)
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
            MeasureOneStateDelay(&o_pin, tv.in_i, tv.in_j, tv.out_i, tv.out_j);
            MeasureOneStateDelay(&o_pin, tv.in_j, tv.in_i, tv.out_j, tv.out_i);
        }
    }
}

void Algorithms::CalculateLogicFunctions(Cell &cell)
{
    for (auto &opin : cell.GetPins(PinDirection::OUT)) {
        auto &log_table = opin.GetLogicTable();
        assert(log_table.size() > 0);

        Expression *fnc_e = new Expression(ExpressionKind::CONSTANT, 0);

        for (const auto &lt_entry : log_table) {
            int output = lt_entry.second;

            Expression* row_e = new Expression(ExpressionKind::CONSTANT, 1);

            // Sum of Products
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

            fnc_e = new Expression(ExpressionKind::OR, fnc_e, row_e);
        }

        fnc_e->Simplify();
        opin.SetLogicFunction(fnc_e);
        opin.PrintLogicFunction();
    }
}

void Algorithms::CharacterizeCells(Cell &cell)
{
    MeasureLogicTableAndLeakage(cell);
    CalculateLogicFunctions(cell);
    MeasureComboDelay(cell);
}

}
