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

#ifndef STIMULUS_H
#define STIMULUS_H

#include <vector>
#include <string>

#include "open_char.h"

namespace open_char {

class Stimulus {

    public:
        StimKind kind_;

        // Constant source
        Volt volage_;

        // Pulse source
        Volt v1_;
        Volt v2_;

        NanoSecond t_delay_;
        NanoSecond t_rise_;
        NanoSecond t_fall_;
        NanoSecond pulse_width_;
        NanoSecond period_;
        int num_pulses_;

        // Piece-wise linear
        std::vector<std::pair<Volt,NanoSecond>> pwl_vals_;

        Stimulus(Volt voltage);
        Stimulus(Volt v1, Volt v2, NanoSecond t_delay, NanoSecond t_rise,
                 NanoSecond t_fall, NanoSecond pulse_width, NanoSecond period,
                 int num_pulses);
        Stimulus(std::vector<std::pair<Volt,NanoSecond>> pwl_vals);
};

}

#endif