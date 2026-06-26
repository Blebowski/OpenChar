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

#ifndef OPCOND_H
#define OPCOND_H

#include <utility>

#include "open_char.h"
#include "Supply.h"

namespace open_char {

class OpCond {

    public:
        OpCond(Supply *supply);

        void SetName(std::string name);
        const std::string& GetName();

        void SetTemperature(Celsius temperature);
        Celsius GetTemperature();

        Supply* GetSupply();
        void SetSupply(Supply *supply);

        void WriteLiberty(FILE *f, size_t tab);

    private:
        std::string name_;
        Supply *supply_;
        Celsius temperature_;
};

}

#endif