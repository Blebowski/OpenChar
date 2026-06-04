
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"

using namespace open_char;

static void test_and2(Context &ctx, Algorithms &algs)
{
    CREATE_AND2_CELL(ctx, c1)

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableLeakageSims(c1));
    algs.MeasureComboLogicTables(c1);
    algs.CalculateComboLogicFunctions(c1);

    Expression *e = c1.GetPin("Z").GetLogicFunction();
    assert (e != nullptr);
    assert (e->GetKind() == ExpressionKind::AND);

    Expression *lhs = e->GetLhs();
    Expression *rhs = e->GetRhs();

    assert (lhs != nullptr);
    assert (rhs != nullptr);

    assert (lhs->GetKind() == ExpressionKind::TERM);
    assert (rhs->GetKind() == ExpressionKind::TERM);
    assert (lhs->GetPin() == &(c1.GetPin("A")));
    assert (rhs->GetPin() == &(c1.GetPin("B")));
}

static void test_half_adder(Context &ctx, Algorithms &algs)
{
    CREATE_HALF_ADDER_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableLeakageSims(c1));
    algs.MeasureComboLogicTables(c1);
    algs.CalculateComboLogicFunctions(c1);

    // S: A ^ B
    Expression *e = c1.GetPin("S").GetLogicFunction();

    assert (e->GetKind() == ExpressionKind::XOR);
    Expression *l1 = e->GetLhs();
    Expression *r1 = e->GetRhs();

    assert (l1 != nullptr);
    assert (r1 != nullptr);

    assert (l1->GetKind() == ExpressionKind::TERM);
    assert (r1->GetKind() == ExpressionKind::TERM);

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