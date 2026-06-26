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

#include "Supply.h"

namespace open_char {

Supply::Supply(std::string vdd_name, Volt vdd_val, std::string gnd_name, Volt gnd_val) :
    vdd_name_(vdd_name),
    gnd_name_(gnd_name),
    vdd_val_(vdd_val),
    gnd_val_(gnd_val)
{}

void Supply::SetGndName(std::string &name)
{
    gnd_name_ = name;
}

void Supply::SetVddName(std::string &name)
{
    vdd_name_ = name;
}

void Supply::SetGndVoltage(Volt value)
{
    gnd_val_ = value;
}

void Supply::SetVddVoltage(Volt value)
{
    vdd_val_ = value;
}

std::string Supply::GetGndName()
{
    return gnd_name_;
}

Volt Supply::GetGndVoltage()
{
    return gnd_val_;
}

std::string Supply::GetVddName()
{
    return vdd_name_;
}

Volt Supply::GetVddVoltage()
{
    return vdd_val_;
}

}