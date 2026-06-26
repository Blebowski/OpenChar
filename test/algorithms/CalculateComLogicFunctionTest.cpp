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

#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"

using namespace open_char;

static void test_and2(Context &ctx, Algorithms &algs)
{
    CREATE_AND2_CELL(ctx, c1)

    RUN_SIMULATIONS(ctx, algs.PrepareComLogicTablesSims(c1));
    algs.MeasureComLogicTables(c1);
    algs.CalculateComLogicFunctions(c1);

    Expression *e = c1.GetPin("Z").GetLogicFunction();
    assert (e != nullptr);
    assert (e->GetKind() == ExprKind::AND);

    Expression *lhs = e->GetLhs();
    Expression *rhs = e->GetRhs();

    assert (lhs != nullptr);
    assert (rhs != nullptr);

    assert (lhs->GetKind() == ExprKind::TERM);
    assert (rhs->GetKind() == ExprKind::TERM);
    assert (lhs->GetPin() == &(c1.GetPin("A")));
    assert (rhs->GetPin() == &(c1.GetPin("B")));
}

static void test_half_adder(Context &ctx, Algorithms &algs)
{
    CREATE_HALF_ADDER_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComLogicTablesSims(c1));
    algs.MeasureComLogicTables(c1);
    algs.CalculateComLogicFunctions(c1);

    // S: A ^ B
    Expression *e = c1.GetPin("S").GetLogicFunction();

    assert (e->GetKind() == ExprKind::XOR);
    Expression *l1 = e->GetLhs();
    Expression *r1 = e->GetRhs();

    assert (l1 != nullptr);
    assert (r1 != nullptr);

    assert (l1->GetKind() == ExprKind::TERM);
    assert (r1->GetKind() == ExprKind::TERM);

    assert (l1->GetPin() == &c1.GetPin("A"));
    assert (r1->GetPin() == &c1.GetPin("B"));
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_and2       (ctx, algs);
    test_half_adder (ctx, algs);

    return 0;
}