

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
    if (abs(val - op_cond.supply_->GetVddVoltage()) < 0.01)
        return 1;
    return 0;
}

int Algorithms::GetBit(int64_t v, size_t index)
{
    return (v >> index) & 0x1;
}

void Algorithms::MeasureLogicFunction(Cell &cell)
{
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();
    Volt log0_v = op_cond.supply_->GetGndVoltage();
    Volt log1_v = op_cond.supply_->GetVddVoltage();

    auto o_pins = cell.GetPins(PinDirection::OUT);

    for (auto & o_pin : o_pins) {
        auto i_pins = cell.GetPins(PinDirection::IN);
        size_t i_pins_len = cell.GetPinsCount(PinDirection::IN);

        size_t n_sims = 1;
        for (size_t i = 0; i < i_pins_len; i++)
            n_sims *= 2;

        for (size_t ipin_vect = 0; ipin_vect < n_sims; ipin_vect++) {

            std::string sim_name = cell.GetName() + "_LOG_FNC";
            size_t input = ipin_vect;

            for (const auto & i_pin : i_pins) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, input & 0x1);
                input >>= 1;
            }

            Simulation sim {sim_name, &cell, SimulationKind::DC};

            sim.SetTemp(op_cond.temp_);
            sim.SetSupply(op_cond.supply_);

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

            o_pin.AddLogicTableEntry(ipin_vect, ToLogic(w.GetDataAtIndex(o_pin.name_, 0)));
        }
    }

    for (auto & o_pin : o_pins) {
        o_pin.PrintLogicTable();
    }
}

NanoSecond Algorithms::FindEdge(Waves &w, Pin *pin, int from)
{
    size_t len = w.GetDataLen();
    const std::vector<Volt>& d = w.GetData(pin->name_);

    // TODO: Cross-check first and last data match the "from" and "to".
    // TODO: Add support for configurable threshold
    Volt th = 0.5 * ctx_->GetLibrary().GetOpCond().supply_->GetVddVoltage();

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

    // TODO: Move this down to Waves
    const std::vector<NanoSecond>& t = w.GetData("time");
    return t[index];
}

int Algorithms::MeasureOneStateDelay(Pin *opin, int64_t in_from, int64_t in_to,
                                     int out_from, int out_to)
{
    Cell *cell = opin->cell_;
    auto i_pins = cell->GetPins(PinDirection::IN);
    OpCond &op_cond = ctx_->GetLibrary().GetOpCond();

    std::string prefix = cell->GetName() + "_DLY";
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

            Simulation sim {sim_name, cell, SimulationKind::TRAN};
            sim.SetTemp(op_cond.temp_);
            sim.SetSupply(op_cond.supply_);

            for (const auto & netlist : ctx_->GetNetlists())
                sim.AddInclude(netlist);

            for (const auto & model : ctx_->GetModels())
                sim.AddModel(model);

            Volt log0_v = op_cond.supply_->GetGndVoltage();
            Volt log1_v = op_cond.supply_->GetVddVoltage();

            int i = 0;
            Pin *tran_pin = nullptr;
            int tran_from = 0;

            for (auto & i_pin : i_pins) {

                int from = GetBit(in_from, i);
                int to = GetBit(in_to, i);

                if (from == to) {
                    sim.AddStimuli((Pin*)&i_pin, Stimulus((from == 1) ? log1_v : log0_v));
                } else {
                    // TODO: Refine
                    Stimulus edge (
                        (from == 1) ? log1_v : log0_v,
                         ( to == 1) ? log1_v : log0_v,
                         1,
                         in_tran,
                         in_tran,
                         10,
                         10,
                         1
                    );
                    sim.AddStimuli((Pin*)&i_pin, std::move(edge));
                    tran_pin = &i_pin;
                    tran_from = from;
                }

                i++;
            }

            sim.AddLoad(opin, out_cap);

            // TODO: Propagate error
            sim.Simulate();
            Waves w = sim.ReadWaves();

            assert(tran_pin != nullptr);
            NanoSecond in_edge  = FindEdge(w, tran_pin, tran_from);
            NanoSecond out_edge = FindEdge(w, opin, out_from);

            delay_table.AddDelay(i_tran, out_edge - in_edge);
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

void Algorithms::CharacterizeCells(Cell &cell)
{
    MeasureLogicFunction(cell);
    MeasureComboDelay(cell);
}

}
