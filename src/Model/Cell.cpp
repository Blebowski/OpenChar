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

#include <algorithm>

#include "open_char.h"
#include "Cell.h"
#include "Utils.h"
#include "Simulation.h"

namespace open_char {

Cell::Cell(std::string name, Library *library) :
    name_(name),
    library_(library),
    d_template_(nullptr),
    c_template_(nullptr),
    area_(0.0),
    footprint_(""),
    supply_(nullptr),
    seq_(this),
    charact_state_(CharactState::START)
{};

Cell::~Cell()
{
    for (const auto & lkg : leakage_table_)
        delete lkg.first;
}

Library* Cell::GetLibrary()
{
    assert(library_ != nullptr);
    return library_;
}

const std::string& Cell::GetName()
{
    return name_;
}

CellKind Cell::GetKind()
{
    return kind_;
}

void Cell::SetKind(CellKind kind)
{
    kind_ = kind;
}

void Cell::SetArea(double area)
{
    area_ = area;
}

double Cell::GetArea()
{
    return area_;
}

void Cell::SetFootprint(std::string footprint)
{
    footprint_ = footprint;
}

std::string& Cell::GetFootPrint()
{
    return footprint_;
}

void Cell::SetSupply(Supply *supply)
{
    supply_ = supply;
}

Supply* Cell::GetSupply()
{
    assert (supply_ != nullptr);
    return supply_;
}

std::pair<Pin&, bool> Cell::AddPin(std::string name, PinDir direction, PinKind kind)
{
    auto [it, inserted] = pins_.emplace(name, Pin(this, name, direction, kind));
    return {it->second, inserted};
}

Pin& Cell::GetPin(std::string name)
{
    return pins_[name];
}

size_t Cell::GetPinsCount(PinDir direction)
{
    return std::count_if(pins_.cbegin(), pins_.cend(),
                        [direction](const auto & pin){
                            return pin.second.direction_ == direction;
                        });
}

Sequential& Cell::GetSequential()
{
    return seq_;
}

Template* Cell::GetDelayTemplate()
{
    return d_template_;
}

void Cell::SetDelayTemplate(Template *d_template)
{
    d_template_ = d_template;
}

Template* Cell::GetConstraintTemplate()
{
    return c_template_;
}

void Cell::SetConstraintTemplate(Template *c_template)
{
    c_template_ = c_template;
}

void Cell::AddLeakageTableEntry(Expression *e, NanoWatt pwr)
{
    leakage_table_.push_back(std::make_pair(e, pwr));
}

std::vector<std::pair<Expression*, NanoWatt>>& Cell::GetLeakageTable()
{
    return leakage_table_;
}

void Cell::AddSimulation(Simulation *simulation)
{
    simulations_.push_back(simulation);
}

std::vector<Simulation*>& Cell::GetSimulations()
{
    return simulations_;
}

bool Cell::IsSimulationFinished()
{
    // TODO: When there are many simulations in cell, its pins and arcs, checking
    //       if simulation is finished is expensive as each check in Simulation
    //       object involves mutex lock / unlock. Later this may need to be optimized

    for (auto & sim : simulations_) {
        if (!sim->IsFinished())
            return false;
    }

    for (auto & pin : pins_) {
        for (auto & sim : pin.second.GetSimulations()) {
            if (!sim->IsFinished())
                return false;
        }
        // TODO: Move some of this down to Arc
        for (auto & arc : pin.second.GetArcs()) {
            for (auto & sims_row : arc.GetSimulations()) {
                for (auto & sims_row_col : sims_row) {
                    for (Simulation *sim : sims_row_col) {
                        if (!sim->IsFinished()) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void Cell::SetCharactState(CharactState charact_state)
{
    charact_state_ = charact_state;
}

CharactState Cell::GetCharactState()
{
    return charact_state_;
}

void Cell::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "cell (%s) {\n", name_);
    tab++;

    TAB_FPRINTF(tab, f, "area: %f\n", area_);
    if (footprint_ != "") {
        TAB_FPRINTF(tab, f, "cell_footprint: %s\n", footprint_);
    }

    for (auto & lkg : leakage_table_) {
        TAB_FPRINTF(tab, f, "leakage_power () {\n");
        tab++;
        TAB_FPRINTF(tab, f, "value : %f;\n", lkg.second);
        TAB_FPRINTF(tab, f, "when : \"");
        lkg.first->Print(f);
        fprintf(f, "\";\n");
        tab--;
        TAB_FPRINTF(tab, f, "} /* end leakage_power */\n");
    }

    if (kind_ == CellKind::SEQUENTIAL) {
        seq_.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::PWR)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::GND)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::CLK)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::ASYNC)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::DATA)) {
        pin.WriteLiberty(f, tab);
    }

    tab--;
    TAB_FPRINTF(tab, f, "} /* end cell */\n");
}

void Cell::WriteVerilog(FILE *f)
{
    TAB_FPRINTF(0, f, "`celldefine\n");
    TAB_FPRINTF(0, f, "module %s (", name_);

    bool first = true;
    for (auto &pin : GetPins()) {
        TAB_FPRINTF(0, f, "%s%s", (first) ? "" : ",", pin.name_);
        first = false;
    }
    TAB_FPRINTF(0, f, ");\n");

    for (auto &pin : GetPins()) {
        pin.WriteVerilog(f, 2);
    }

    switch (kind_) {
    case CellKind::COMBINATIONAL:
    {
        for (auto & o_pin : GetPins(PinDir::OUT)) {
            TAB_FPRINTF(2, f, "assign %s = ", o_pin.name_);
            o_pin.GetLogicFunction()->Print(f);
            TAB_FPRINTF(0, f, ";\n");
        }
        break;
    }
    default:
        fatal("Unhandled CellKind: %s", toString(kind_));
        break;
    }

    TAB_FPRINTF(0, f, "endmodule\n");
    TAB_FPRINTF(0, f, "`endcelldefine\n\n");
}

}