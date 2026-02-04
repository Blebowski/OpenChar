

#include "Algorithms.h"
#include "Simulation.h"
#include "Library.h"
#include "Utils.h"

namespace open_char {

Algorithms::Algorithms(Context *ctx) :
    ctx_(ctx)
{}

int Algorithms::ToLogic(double val)
{
    OpCond& op_cond = ctx_->lib_.GetOpCond();
    if (abs(val - op_cond.supply_->vdd_val_) < 0.05)
        return 1;
    return 0;
}

bool Algorithms::GetLogicFunction(Cell &cell)
{
    OpCond &op_cond = ctx_->lib_.GetOpCond();
    double log0_v = op_cond.supply_->gnd_val_;
    double log1_v = op_cond.supply_->vdd_val_;

    auto o_pins = cell.GetPins(PinDirection::OUT);

    for (auto & o_pin : o_pins) {
        auto i_pins = cell.GetPins(PinDirection::IN);
        size_t i_pins_len = cell.GetPinsCount(PinDirection::IN);

        size_t n_sims = 1;
        for (size_t i = 0; i < i_pins_len; i++)
            n_sims *= 2;

        for (size_t ipin_vect = 0; ipin_vect < n_sims; ipin_vect++) {

            std::string sim_name = cell.name_ + "_GET_LOGIC_FUNC";
            size_t input = ipin_vect;

            for (const auto & i_pin : i_pins) {
                sim_name = sprintf("%s_%s%d", sim_name, i_pin.name_, input & 0x1);
                input >>= 1;
            }

            Simulation sim {sim_name, &cell};

            sim.SetTemp(op_cond.temp_);
            sim.SetSupply(op_cond.supply_);

            for (const auto & inc : ctx_->includes_)
                sim.AddInclude(inc);

            for (const auto & lib : ctx_->libs_)
                sim.AddLib(lib);

            int i_pin_index = 0;
            for (const auto & i_pin : i_pins) {
                size_t i_pin_val = ((ipin_vect >> i_pin_index) & 0x1);
                sim.AddStimuli((Pin*)&i_pin, Stimulus((i_pin_val == 1) ? log1_v : log0_v));

                i_pin_index++;
            }

            sim.Simulate();

            Waves w = sim.ReadWaves();
            w.Print();

            o_pin.AddLogicTableEntry(ipin_vect, ToLogic(w.data_[o_pin.name_][0]));
        }
    }

    for (auto & o_pin : o_pins) {
        o_pin.PrintLogicTable();
    }

    return true;
}

}
