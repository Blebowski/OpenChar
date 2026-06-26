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

#include <bits/stdc++.h>

#include "Context.h"
#include "Library.h"
#include "Utils.h"

namespace open_char {

Library::Library(Context *ctx) :
    supplies_({{"VDD", 1.2, "VSS", 0}}),
    op_cond_(&supplies_[0]),
    ctx_(ctx)
{}

std::pair<Cell&, bool> Library::AddCell(std::string name)
{
    auto [it, inserted] = cells_.emplace(name, Cell(name, this));
    return {it->second, inserted};
}

Cell& Library::GetCell(std::string name)
{
    return cells_.find(name)->second;
}

std::map<std::string, Cell>& Library::GetCells()
{
    return cells_;
}

bool Library::HasCell(std::string name)
{
    if (cells_.contains(name))
        return true;
    return false;
}

OpCond& Library::GetOpCond()
{
    return op_cond_;
}

void Library::SetDefaultSupplyVdd(std::string name, double val)
{
    supplies_[0].SetVddName(name);
    supplies_[0].SetVddVoltage(val);
}

void Library::SetDefaultSupplyVdd(double val)
{
    supplies_[0].SetVddVoltage(val);
}

void Library::SetDefaultSupplyGnd(std::string name, double val)
{
    supplies_[0].SetGndName(name);
    supplies_[0].SetGndVoltage(val);
}

bool Library::HasSupply(std::string vdd_name)
{
    for (auto &sup : supplies_) {
        if (sup.GetVddName() == vdd_name)
            return true;
    }
    return false;
}

Supply& Library::GetSupply(std::string vdd_name)
{
    for (auto &sup : supplies_) {
        if (sup.GetVddName() == vdd_name)
            return sup;
    }
    return supplies_[0];
}

std::pair<Template&, bool> Library::AddTemplate(std::string name)
{
    auto [it, inserted] = templates_.emplace(name, Template(name));
    return {it->second, inserted};
}

bool Library::HasTemplate(std::string name)
{
    if (templates_.contains(name))
        return true;
    return false;
}

Template& Library::GetTemplate(std::string name)
{
    return templates_.find(name)->second;
}

void Library::WriteLiberty(const std::string &name)
{
    FILE *f = fopen(name.c_str(), "w");

    TAB_FPRINTF(0, f, "library (%s) {\n", name);

    size_t tab = 1;

    // TODO: Make units configurable!
    TAB_FPRINTF(tab, f, "technology : cmos ;\n");
    TAB_FPRINTF(tab, f, "delay_mode : table_lookup ;\n");
    TAB_FPRINTF(tab, f, "capacitive_load_unit (1,pf);\n");
    TAB_FPRINTF(tab, f, "current_unit : \"1nA\";\n");
    TAB_FPRINTF(tab, f, "leakage_power_unit : \"1nW\";\n");
    TAB_FPRINTF(tab, f, "pulling_resistance_unit : \"1kohm\";\n");
    TAB_FPRINTF(tab, f, "time_unit : \"1ns\";\n");
    TAB_FPRINTF(tab, f, "voltage_unit : \"1V\";\n");

    Variables& v = ctx_->GetVariables();

    TAB_FPRINTF(tab, f, "input_threshold_pct_fall : %.0f;\n",
                        std::round(v.GetDoubleVariable("delay_in_fall") * 100.0));
    TAB_FPRINTF(tab, f, "input_threshold_pct_rise : %.0f;\n",
                        std::round(v.GetDoubleVariable("delay_in_rise") * 100.0));
    TAB_FPRINTF(tab, f, "output_threshold_pct_fall : %.0f;\n",
                        std::round(v.GetDoubleVariable("delay_out_fall") * 100.0));
    TAB_FPRINTF(tab, f, "output_threshold_pct_rise : %.0f;\n",
                        std::round(v.GetDoubleVariable("delay_out_rise") * 100.0));

    TAB_FPRINTF(tab, f, "slew_lower_threshold_pct_fall : %.0f;\n",
                        std::round(v.GetDoubleVariable("slew_lower_fall") * 100.0));
    TAB_FPRINTF(tab, f, "slew_lower_threshold_pct_rise : %.0f;\n",
                        std::round(v.GetDoubleVariable("slew_lower_rise") * 100.0));
    TAB_FPRINTF(tab, f, "slew_upper_threshold_pct_fall : %.0f;\n",
                        std::round(v.GetDoubleVariable("slew_upper_fall") * 100.0));
    TAB_FPRINTF(tab, f, "slew_upper_threshold_pct_rise : %.0f;\n",
                        std::round(v.GetDoubleVariable("slew_upper_rise") * 100.0));

    op_cond_.WriteLiberty(f, tab);

    for (auto & [cell_name, cell] : cells_) {
        cell.WriteLiberty(f, tab);
    }

    TAB_FPRINTF(0, f, "} /* end library */\n", name);
}

void Library::WriteVerilog(const std::string &name)
{
    FILE *f = fopen(name.c_str(), "w");

    // TODO: Make timescale configurable
    TAB_FPRINTF(0, f, "`timescale 1ns/1ps\n\n", name);

    for (auto & [cell_name, cell] : cells_) {
        cell.WriteVerilog(f);
    }

    fclose(f);
}

}
