////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>

#include "open_char.h"
#include "Stimulus.h"

namespace open_char {

    Stimulus::Stimulus(Volt voltage) :
        kind_(StimKind::CONSTANT),
        volage_(voltage)
    {};

    Stimulus::Stimulus(Volt v1, Volt v2, NanoSecond t_delay, NanoSecond t_rise,
                       NanoSecond t_fall, NanoSecond pulse_width, NanoSecond period,
                       int num_pulses):
        kind_(StimKind::PULSE),
        v1_(v1),
        v2_(v2),
        t_delay_(t_delay),
        t_rise_(t_rise),
        t_fall_(t_fall),
        pulse_width_(pulse_width),
        period_(period),
        num_pulses_(num_pulses)
    {};

    Stimulus::Stimulus(std::vector<std::pair<Volt,NanoSecond>> pwl_vals) :
        kind_(StimKind::PWL),
        pwl_vals_(pwl_vals)
    {};

}
