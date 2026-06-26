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

static void test_nand2(Context &ctx, Algorithms &algs)
{
    CREATE_NAND2_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareLeakageSims(c1));
    algs.MeasureLeakage(c1);

    // leakage_power () {
    //   value : 0.016880;
    //   when : "!A & !B";
    // } /* end leakage_power */

    auto [e0, lkg0] = c1.GetLeakageTable()[0];

    assert(e0->GetKind() == ExprKind::AND);
    Expression *e0_lhs = e0->GetLhs();
    Expression *e0_rhs = e0->GetRhs();
    assert(e0_lhs->GetKind() == ExprKind::NOT);
    assert(e0_rhs->GetKind() == ExprKind::NOT);
    Expression *e0_lhs_lhs = e0_lhs->GetLhs();
    Expression *e0_rhs_lhs = e0_rhs->GetLhs();
    assert(e0_lhs_lhs->GetKind() == ExprKind::TERM);
    assert(e0_lhs_lhs->GetPin() == &c1.GetPin("A"));
    assert(e0_rhs_lhs->GetKind() == ExprKind::TERM);
    assert(e0_rhs_lhs->GetPin() == &c1.GetPin("B"));

    CHECK_FLOAT(lkg0, 0.016880);

    // leakage_power () {
    //   value : 0.059710;
    //   when : "A & !B";
    // } /* end leakage_power */

    auto [e1, lkg1] = c1.GetLeakageTable()[1];

    assert(e1->GetKind() == ExprKind::AND);
    Expression *e1_lhs = e1->GetLhs();
    Expression *e1_rhs = e1->GetRhs();
    assert(e1_lhs->GetKind() == ExprKind::TERM);
    assert(e1_lhs->GetPin() == &c1.GetPin("A"));
    assert(e1_rhs->GetKind() == ExprKind::NOT);
    Expression *e1_rhs_lhs = e1_rhs->GetLhs();
    assert(e1_rhs_lhs->GetKind() == ExprKind::TERM);
    assert(e1_rhs_lhs->GetPin() == &c1.GetPin("B"));

    CHECK_FLOAT(lkg1, 0.059710);

    // leakage_power () {
    //   value : 0.061408;
    //   when : "!A & B";
    // } /* end leakage_power */

    auto [e2, lkg2] = c1.GetLeakageTable()[2];

    assert(e2->GetKind() == ExprKind::AND);
    Expression *e2_lhs = e2->GetLhs();
    Expression *e2_rhs = e2->GetRhs();
    assert(e2_lhs->GetKind() == ExprKind::NOT);
    assert(e2_rhs->GetKind() == ExprKind::TERM);
    assert(e2_rhs->GetPin() == &c1.GetPin("B"));
    Expression *e2_lhs_lhs = e2_lhs->GetLhs();
    assert(e2_lhs_lhs->GetKind() == ExprKind::TERM);
    assert(e2_lhs_lhs->GetPin() == &c1.GetPin("A"));

    CHECK_FLOAT(lkg2, 0.061408);

    // leakage_power () {
    //   value : 0.134034;
    //   when : "A & B";
    // } /* end leakage_power */

    auto [e3, lkg3] = c1.GetLeakageTable()[3];

    assert(e3->GetKind() == ExprKind::AND);
    Expression *e3_lhs = e3->GetLhs();
    Expression *e3_rhs = e3->GetRhs();
    assert(e3_lhs->GetKind() == ExprKind::TERM);
    assert(e3_lhs->GetPin() == &c1.GetPin("A"));
    assert(e3_rhs->GetKind() == ExprKind::TERM);
    assert(e3_rhs->GetPin() == &c1.GetPin("B"));

    CHECK_FLOAT(lkg3, 0.134034);

}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_nand2(ctx, algs);

    return 0;
}