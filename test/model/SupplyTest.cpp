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

#include <cassert>
#include <filesystem>

#include "Supply.h"
#include "Utils.h"

using namespace open_char;

int main()
{
    Supply *s1 = new Supply("VDDIO", 0.7, "VSSIO", 0.0);

    assert(s1->GetVddName()      == "VDDIO");
    assert(s1->GetGndName()      == "VSSIO");
    assert(COMPARE_FLOATS(s1->GetVddVoltage(), 0.7));
    assert(COMPARE_FLOATS(s1->GetGndVoltage(), 0.0));

    std::string vss = "VSS";
    std::string vdd = "VDD";
    s1->SetGndName(vss);
    s1->SetVddName(vdd);
    s1->SetGndVoltage(0.5);
    s1->SetVddVoltage(1.9);

    assert (s1->GetGndName() == "VSS");
    assert (s1->GetVddName() == "VDD");
    assert(COMPARE_FLOATS(s1->GetVddVoltage(), 1.9));
    assert(COMPARE_FLOATS(s1->GetGndVoltage(), 0.5));

    delete s1;
}