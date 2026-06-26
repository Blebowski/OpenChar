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

#include "OpCond.h"
#include "Utils.h"

namespace open_char {

OpCond::OpCond(Supply *supply) :
    name_("DEFAULT_OPCOND"),
    supply_(supply),
    temperature_(25.0)
{}

void OpCond::SetName(std::string name)
{
    name_ = name;
}

const std::string& OpCond::GetName()
{
    return name_;
}

void OpCond::SetTemperature(Celsius temperature)
{
    temperature_ = temperature;
}

Celsius OpCond::GetTemperature()
{
    return temperature_;
}

Supply* OpCond::GetSupply()
{
    assert (supply_ != nullptr);
    return supply_;
}

void OpCond::SetSupply(Supply *supply)
{
    supply_ = supply;
}

void OpCond::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "operating conditions (%s) {\n", name_);

    tab++;
    TAB_FPRINTF(tab, f, "process : 1.00 ;\n");
    TAB_FPRINTF(tab, f, "voltage : %f ;\n", supply_->GetVddVoltage());
    TAB_FPRINTF(tab, f, "temperature : %f ;\n", temperature_);
    tab--;

    TAB_FPRINTF(tab, f, "} /* end operating_conditions */\n");
}

}