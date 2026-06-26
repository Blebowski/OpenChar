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

#ifndef WAVES_H
#define WAVES_H

#include <map>
#include <string>
#include <vector>
#include <ctime>

#include "open_char.h"

namespace open_char {

class Waves {

    public:
        Waves(std::string path);
        void Print();

        std::string title_;
        std::string plot_name_;

        const std::vector<Volt>& GetVoltage(const std::string &node_name);
        const std::vector<MicroAmp>& GetCurrent(const std::string &node_name);

        size_t FindTransitionIndex(std::string name, Volt th);
        NanoSecond FindTransitionTime(std::string name, Volt th);

        size_t FindTransitionIndex(std::string name, Volt th,
                                   NanoSecond time_start, NanoSecond time_end);
        NanoSecond FindTransitionTime(std::string name, Volt th,
                                      NanoSecond time_start, NanoSecond time_end);

        NanoSecond GetTimeAtIndex(size_t index);
        size_t GetIndexOfTime(NanoSecond time);

    private:
        size_t FindReferenceIndex(double val);
        std::map<std::string, std::pair<size_t, std::vector<Volt>>> voltages_;
        std::map<std::string, std::pair<size_t, std::vector<MicroAmp>>> currents_;
        std::vector<double> reference_;
        WaveKind kind_;
};

}

#endif