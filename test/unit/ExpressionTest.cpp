
#include <cassert>

#include "Expression.h"
#include "Pin.h"

using namespace open_char;

static Pin *make_pin(const char *name)
{
    return new Pin(nullptr, name, PinDir::IN, PinKind::DATA);
}

void test_construction_and_getters()
{
    // CONSTANT
    {
        Expression *e = new Expression(ExprKind::CONSTANT, 1);
        assert(e->GetKind() == ExprKind::CONSTANT);
        assert(e->GetConstValue() == 1);
        delete e;
    }
    {
        Expression *e = new Expression(ExprKind::CONSTANT, 0);
        assert(e->GetConstValue() == 0);
        delete e;
    }

    // TERM
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::TERM, pa);
        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pa);
        delete e;
        delete pa;
    }

    // NOT
    {
        Pin *pa = make_pin("A");
        Expression *inner = new Expression(ExprKind::TERM, pa);
        Expression *e = new Expression(ExprKind::NOT, inner);
        assert(e->GetKind() == ExprKind::NOT);
        assert(e->GetLhs() == inner);
        delete e;
        delete pa;
    }

    // AND, OR, XOR
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e_and = new Expression(ExprKind::AND,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));
        assert(e_and->GetKind() == ExprKind::AND);
        delete e_and;

        Expression *e_or = new Expression(ExprKind::OR,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));
        assert(e_or->GetKind() == ExprKind::OR);
        delete e_or;

        Expression *e_xor = new Expression(ExprKind::XOR,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));
        assert(e_xor->GetKind() == ExprKind::XOR);
        delete e_xor;

        delete pa;
        delete pb;
    }
}

void test_equals()
{
    // EQUAL: A & B == B & A (truth-table comparison)
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e1 = new Expression(ExprKind::AND,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));
        Expression *e2 = new Expression(ExprKind::AND,
            new Expression(ExprKind::TERM, pb),
            new Expression(ExprKind::TERM, pa));

        assert(e1->Equals(e2, ExprEqualKind::EQUAL));
        assert(!e1->Equals(e2, ExprEqualKind::INVERT));

        delete e1;
        delete e2;
        delete pa;
        delete pb;
    }

    // INVERT: A != !A
    {
        Pin *pa = make_pin("A");

        Expression *e1 = new Expression(ExprKind::TERM, pa);
        Expression *e2 = new Expression(ExprKind::NOT,
            new Expression(ExprKind::TERM, pa));

        assert(e1->Equals(e2, ExprEqualKind::INVERT));
        assert(!e1->Equals(e2, ExprEqualKind::EQUAL));

        delete e1;
        delete e2;
        delete pa;
    }

    // EQUAL: A | B == B | A (commutativity via truth table)
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e1 = new Expression(ExprKind::OR,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));
        Expression *e2 = new Expression(ExprKind::OR,
            new Expression(ExprKind::TERM, pb),
            new Expression(ExprKind::TERM, pa));

        assert(e1->Equals(e2, ExprEqualKind::EQUAL));

        delete e1;
        delete e2;
        delete pa;
        delete pb;
    }

    // Not equal: A & B != A | B
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e1 = new Expression(ExprKind::AND,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));
        Expression *e2 = new Expression(ExprKind::OR,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));

        assert(!e1->Equals(e2, ExprEqualKind::EQUAL));
        assert(!e1->Equals(e2, ExprEqualKind::INVERT));

        delete e1;
        delete e2;
        delete pa;
        delete pb;
    }
}

void test_substitute()
{
    // Substitute A=1 into (A & B), then Simplify → B
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e = new Expression(ExprKind::AND,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));

        e->Substitute(pa, 1);
        e->Simplify();

        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pb);

        delete e;
        delete pa;
        delete pb;
    }

    // Substitute A=0 into (A | B), then Simplify → B
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e = new Expression(ExprKind::OR,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::TERM, pb));

        e->Substitute(pa, 0);
        e->Simplify();

        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pb);

        delete e;
        delete pa;
        delete pb;
    }
}

void test_simplify_const_fold()
{
    // 1 & A → A
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::AND,
            new Expression(ExprKind::CONSTANT, 1),
            new Expression(ExprKind::TERM, pa));
        e->Simplify();
        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pa);
        delete e;
        delete pa;
    }

    // A & 1 → A
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::AND,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::CONSTANT, 1));
        e->Simplify();
        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pa);
        delete e;
        delete pa;
    }

    // 0 | A → A
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::OR,
            new Expression(ExprKind::CONSTANT, 0),
            new Expression(ExprKind::TERM, pa));
        e->Simplify();
        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pa);
        delete e;
        delete pa;
    }

    // A | 0 → A
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::OR,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::CONSTANT, 0));
        e->Simplify();
        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pa);
        delete e;
        delete pa;
    }

    // 0 ^ A → A
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::XOR,
            new Expression(ExprKind::CONSTANT, 0),
            new Expression(ExprKind::TERM, pa));
        e->Simplify();
        assert(e->GetKind() == ExprKind::TERM);
        assert(e->GetPin() == pa);
        delete e;
        delete pa;
    }
}

void test_simplify_tautology_contradiction()
{
    // A | !A → 1 (tautology)
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::OR,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::NOT,
                new Expression(ExprKind::TERM, pa)));
        e->Simplify();
        assert(e->GetKind() == ExprKind::CONSTANT);
        assert(e->GetConstValue() == 1);
        delete e;
        delete pa;
    }

    // A & !A → 0 (contradiction)
    {
        Pin *pa = make_pin("A");
        Expression *e = new Expression(ExprKind::AND,
            new Expression(ExprKind::TERM, pa),
            new Expression(ExprKind::NOT,
                new Expression(ExprKind::TERM, pa)));
        e->Simplify();
        assert(e->GetKind() == ExprKind::CONSTANT);
        assert(e->GetConstValue() == 0);
        delete e;
        delete pa;
    }
}

void test_simplify_demorgan()
{
    // !((!A) & (!B)) → A | B
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e = new Expression(ExprKind::NOT,
            new Expression(ExprKind::AND,
                new Expression(ExprKind::NOT, new Expression(ExprKind::TERM, pa)),
                new Expression(ExprKind::NOT, new Expression(ExprKind::TERM, pb))));
        e->Simplify();

        assert(e->GetKind() == ExprKind::OR);
        assert(e->GetLhs()->GetKind() == ExprKind::TERM);
        assert(e->GetRhs()->GetKind() == ExprKind::TERM);

        delete e;
        delete pa;
        delete pb;
    }

    // !((!A) | (!B)) → A & B
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");

        Expression *e = new Expression(ExprKind::NOT,
            new Expression(ExprKind::OR,
                new Expression(ExprKind::NOT, new Expression(ExprKind::TERM, pa)),
                new Expression(ExprKind::NOT, new Expression(ExprKind::TERM, pb))));
        e->Simplify();

        assert(e->GetKind() == ExprKind::AND);
        assert(e->GetLhs()->GetKind() == ExprKind::TERM);
        assert(e->GetRhs()->GetKind() == ExprKind::TERM);

        delete e;
        delete pa;
        delete pb;
    }
}

void test_simplify_associate()
{
    // (A & B) | (A & C) → A & (B | C)
    {
        Pin *pa = make_pin("A");
        Pin *pb = make_pin("B");
        Pin *pc = make_pin("C");

        Expression *e = new Expression(ExprKind::OR,
            new Expression(ExprKind::AND,
                new Expression(ExprKind::TERM, pa),
                new Expression(ExprKind::TERM, pb)),
            new Expression(ExprKind::AND,
                new Expression(ExprKind::TERM, pa),
                new Expression(ExprKind::TERM, pc)));
        e->Simplify();

        assert(e->GetKind() == ExprKind::AND);
        assert(e->GetLhs()->GetKind() == ExprKind::TERM);
        assert(e->GetRhs()->GetKind() == ExprKind::OR);

        delete e;
        delete pa;
        delete pb;
        delete pc;
    }
}

int main()
{
    test_construction_and_getters();
    test_equals();
    test_substitute();
    test_simplify_const_fold();
    test_simplify_tautology_contradiction();
    test_simplify_demorgan();
    test_simplify_associate();
}