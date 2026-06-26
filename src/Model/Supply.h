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

#ifndef SUPPLY_H
#define SUPPLY_H

#include <utility>
#include <string>

#include "open_char.h"

namespace open_char {

class Supply {

    public:
        Supply(std::string vdd_name, Volt vdd_val, std::string gnd_name, Volt gnd_val);

        void SetGndName(std::string &name);
        void SetVddName(std::string &name);
        void SetGndVoltage(Volt value);
        void SetVddVoltage(Volt value);

        std::string GetGndName();
        Volt GetGndVoltage();
        std::string GetVddName();
        Volt GetVddVoltage();

    private:
        std::string vdd_name_;
        std::string gnd_name_;

        Volt vdd_val_;
        Volt gnd_val_;
};

}

#endif