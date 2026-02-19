
#include "open_char.h"
#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include <cassert>

using namespace open_char;

void test_and2(Context &ctx, Algorithms &algs)
{
    ctx.GetLibrary().AddCell("AND2");
    Cell &c1 = ctx.GetLibrary().GetCell("AND2");

    c1.AddPin("Z",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

    algs.PrepareLogicTableAndLeakageSims(c1);
    ctx.GetSimulationPool().StartSimulations();
    ctx.GetSimulationPool().FinishAndProcessSimulations();
    algs.CalculateLogicFunctions(c1);

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

void test_half_adder(Context &ctx, Algorithms &algs)
{
    ctx.GetLibrary().AddCell("HALF_ADDER");
    Cell &c1 = ctx.GetLibrary().GetCell("HALF_ADDER");

    c1.AddPin("CO",  PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("S",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

    algs.PrepareLogicTableAndLeakageSims(c1);
    ctx.GetSimulationPool().StartSimulations();
    ctx.GetSimulationPool().FinishAndProcessSimulations();
    algs.CalculateLogicFunctions(c1);

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
    Context ctx(nullptr);
    Algorithms algs(&ctx);

    ctx.AddNetlist(TEST_COMMON_DIR "/basic_gates.cdl");

    test_and2       (ctx, algs);
    test_half_adder (ctx, algs);

    return 0;
}