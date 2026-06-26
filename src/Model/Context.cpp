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

#include "Context.h"

namespace open_char {

Context::Context(Tcl_Interp* tcl_interp) :
    library_(this),
    algorithms_(this),
    tcl_interp_(tcl_interp),
    sim_pool_(this)
{}

Context::~Context()
{}

Library& Context::GetLibrary()
{
    return library_;
}

Algorithms& Context::GetAlgorithms()
{
    return algorithms_;
}

const std::vector<std::string>& Context::GetModels()
{
    return models_;
}

const std::vector<std::string>& Context::GetNetlists()
{
    return netlists_;
}

void Context::AddModel(const std::string& model)
{
    models_.push_back(model);
}

void Context::AddNetlist(const std::string& netlist)
{
    netlists_.push_back(netlist);
}

void Context::AddTclCommand(TclCmd cmd, Tcl_ObjCmdProc* cb)
{
    tcl_commands_.push_back({cmd, cb});
}

std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>>& Context::GetTclCommands()
{
    return tcl_commands_;
}

Tcl_Interp* Context::GetTclInterp()
{
    assert(tcl_interp_ != nullptr);
    return tcl_interp_;
}

SimulationPool& Context::GetSimulationPool()
{
    return sim_pool_;
}

Variables& Context::GetVariables()
{
    return variables_;
}

}