

#include <fmt/printf.h>

#include "Algorithms.h"
#include "Simulation.h"

namespace open_char {

Algorithms::Algorithms(Context *ctx) :
    ctx_(ctx)
{}

bool Algorithms::GetLogicFunction(Cell &cell)
{
    auto o_pins = cell.GetPins(PinDirection::OUT);
    double log0_v = ctx_->vss_.second;
    double log1_v = ctx_->vcc_.second;

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
                sim_name = fmt::sprintf("%s_%s%d", sim_name, i_pin.name_, input & 0x1);
                input >>= 1;
            }

            Simulation sim {sim_name, &cell};

            sim.SetTemp(ctx_->temp_);
            sim.SetVcc(ctx_->vcc_);
            sim.SetVss(ctx_->vss_);

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

            // TODO: Add logic_table_ entry here
        }
    }

    // TODO: Extract the logic function!

    return true;
}

}
