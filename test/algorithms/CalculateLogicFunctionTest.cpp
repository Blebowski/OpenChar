
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

    algs.MeasureLogicFunction(c1);
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

    algs.MeasureLogicFunction(c1);
    algs.CalculateLogicFunctions(c1);

    //    <--11-->   <--r1-->
    //    l1l1 l1r1  r1l1  r1r1
    // S: (A & !B) | (!A & B)
    Expression *e = c1.GetPin("S").GetLogicFunction();

    assert (e->GetKind() == ExpressionKind::OR);
    Expression *l1 = e->GetLhs();
    Expression *r1 = e->GetRhs();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // r1
    ///////////////////////////////////////////////////////////////////////////////////////////////
    assert (r1->GetKind() == ExpressionKind::AND);

    Expression *r1r1 = r1->GetRhs();
    assert (r1r1->GetKind() == ExpressionKind::TERM);
    assert (r1r1->GetPin() == &c1.GetPin("B"));

    Expression *r1l1 = r1->GetLhs();
    assert (r1l1->GetKind() == ExpressionKind::NOT);
    Expression *r1l1l1 = r1l1->GetLhs();
    assert (r1l1l1->GetKind() == ExpressionKind::TERM);
    assert (r1l1l1->GetPin() == &c1.GetPin("A"));

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // l1
    ///////////////////////////////////////////////////////////////////////////////////////////////
    assert (l1->GetKind() == ExpressionKind::AND);

    Expression *l1r1 = l1->GetRhs();
    assert (l1r1->GetKind() == ExpressionKind::NOT);
    Expression *l1r1l1 = l1r1->GetLhs();
    assert (l1r1l1->GetKind() == ExpressionKind::TERM);
    assert (l1r1l1->GetPin() == &c1.GetPin("B"));

    Expression *l1l1 = l1->GetLhs();
    assert (l1l1->GetKind() == ExpressionKind::TERM);
    assert (l1l1->GetPin() == &c1.GetPin("A"));
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