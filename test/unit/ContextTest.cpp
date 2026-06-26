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

#include "Context.h"
#include "Library.h"
#include "Algorithms.h"

int main()
{
    using namespace open_char;

    Context *ctx = new Context(nullptr);

    [[maybe_unused]] Library &lib = ctx->GetLibrary();
    [[maybe_unused]] Algorithms &algos = ctx->GetAlgorithms();

    ctx->AddModel("ABCDE");
    ctx->AddModel("MY_MODEL");

    assert(ctx->GetModels()[0] == "ABCDE");
    assert(ctx->GetModels()[1] == "MY_MODEL");

    ctx->AddNetlist("PATH TO FANCY NETLIST");
    ctx->AddNetlist("NEXT NETLIST");
    ctx->AddNetlist("ANOTHER ONE");

    assert(ctx->GetNetlists()[0] == "PATH TO FANCY NETLIST");
    assert(ctx->GetNetlists()[1] == "NEXT NETLIST");
    assert(ctx->GetNetlists()[2] == "ANOTHER ONE");

    // Skip testing TCL commands, this will be well tested in end-to-end tests.

    delete ctx;
}