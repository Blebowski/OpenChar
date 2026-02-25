

#include <cassert>

#include "Expression.h"
#include "Utils.h"
#include "Pin.h"

//#define EXPR_DEBUG


namespace open_char {

Expression::Expression(ExpressionKind kind, Expression *lhs, Expression *rhs) :
    kind_(kind),
    lhs_(lhs),
    rhs_(rhs),
    const_val_(0),
    pin_(nullptr)
{
    assert(kind == ExpressionKind::AND ||
           kind == ExpressionKind::OR  ||
           kind == ExpressionKind::XOR);
}

Expression::Expression(ExpressionKind kind, Expression *lhs) :
    kind_(kind),
    lhs_(lhs),
    rhs_(nullptr),
    const_val_(0),
    pin_(nullptr)
{
    assert(kind == ExpressionKind::NOT);
}

Expression::Expression(ExpressionKind kind, Pin *pin) :
    kind_(kind),
    lhs_(nullptr),
    rhs_(nullptr),
    const_val_(0),
    pin_(pin)
{
    assert(kind == ExpressionKind::TERM);
}

Expression::Expression(ExpressionKind kind, int const_val) :
    kind_(kind),
    lhs_(nullptr),
    rhs_(nullptr),
    const_val_(const_val),
    pin_(nullptr)
{
    assert(kind == ExpressionKind::CONSTANT);
}

Expression::~Expression()
{
    if (lhs_ != nullptr)
        delete lhs_;
    if (rhs_ != nullptr)
        delete rhs_;
}

void Expression::Copy(Expression *e)
{
    assert(e != nullptr);
    lhs_ = e->lhs_;
    rhs_ = e->rhs_;
    const_val_ = e->const_val_;
    pin_ = e->pin_;
    kind_ = e->kind_;
}

void Expression::Invalidate()
{
    lhs_ = nullptr;
    rhs_ = nullptr;
}

void Expression::DeMorgan()
{
    if (lhs_ != nullptr) {
        lhs_->DeMorgan();
    }
    if (rhs_ != nullptr) {
        rhs_->DeMorgan();
    }

#ifdef EXPR_DEBUG
    printf("DeMorgan:\n");
    Print(stdout);
    printf("\n");
#endif

    if (kind_ != ExpressionKind::NOT) {
        return;
    }

    assert(lhs_ != nullptr);

    if (lhs_->kind_ == ExpressionKind::AND) {

        assert (lhs_->lhs_ != nullptr);
        assert (lhs_->rhs_ != nullptr);

        if (lhs_->lhs_->kind_ != ExpressionKind::NOT)
            return;
        if (lhs_->rhs_->kind_ != ExpressionKind::NOT)
            return;

        assert(lhs_->lhs_->lhs_ != nullptr);
        assert(lhs_->rhs_->lhs_ != nullptr);

        Expression *lhs_buf = lhs_->lhs_->lhs_;
        Expression *rhs_buf = lhs_->rhs_->lhs_;

        lhs_->lhs_->Invalidate();
        delete lhs_->lhs_;
        lhs_->rhs_->Invalidate();
        delete lhs_->rhs_;
        lhs_->Invalidate();
        delete lhs_;

        kind_ = ExpressionKind::OR;
        lhs_ = lhs_buf;
        rhs_ = rhs_buf;

    } else if (lhs_->kind_ == ExpressionKind::OR) {
        assert (lhs_->lhs_ != nullptr);
        assert (lhs_->rhs_ != nullptr);

        if (lhs_->lhs_->kind_ != ExpressionKind::NOT)
            return;
        if (lhs_->rhs_->kind_ != ExpressionKind::NOT)
            return;

        assert(lhs_->lhs_->lhs_ != nullptr);
        assert(lhs_->rhs_->lhs_ != nullptr);

        Expression *lhs_buf = lhs_->lhs_->lhs_;
        Expression *rhs_buf = lhs_->rhs_->lhs_;

        lhs_->lhs_->Invalidate();
        delete lhs_->lhs_;
        lhs_->rhs_->Invalidate();
        delete lhs_->rhs_;
        lhs_->Invalidate();
        delete lhs_;

        kind_ = ExpressionKind::AND;
        lhs_ = lhs_buf;
        rhs_ = rhs_buf;
    }
}

void Expression::ConstFold()
{
    if (lhs_ != nullptr) {
        lhs_->ConstFold();
    }
    if (rhs_ != nullptr) {
        rhs_->ConstFold();
    }

#ifdef EXPR_DEBUG
    printf("Const Fold:\n");
    Print(stdout);
    printf("\n");
#endif

    switch (kind_) {
    case ExpressionKind::AND:
        assert(lhs_ != nullptr);
        assert(rhs_ != nullptr);
        if (lhs_->kind_ == ExpressionKind::CONSTANT && lhs_->const_val_ == 1) {
            Expression *old = rhs_;
            Copy(rhs_);
            old->Invalidate();
#ifdef EXPR_DEBUG
            printf("FOLDED AND\n");
#endif
            delete old;
        } else if (rhs_->kind_ == ExpressionKind::CONSTANT && rhs_->const_val_ == 1) {
            Expression *old = lhs_;
            Copy(lhs_);
            old->Invalidate();
#ifdef EXPR_DEBUG
            printf("FOLDED AND\n");
#endif
            delete old;
        }
        break;
    case ExpressionKind::OR:
    case ExpressionKind::XOR:
        assert(lhs_ != nullptr);
        assert(rhs_ != nullptr);
        if (lhs_->kind_ == ExpressionKind::CONSTANT && lhs_->const_val_ == 0) {
            Expression *old = rhs_;
            Copy(rhs_);
            old->Invalidate();
#ifdef EXPR_DEBUG
            printf("FOLDED OR/XOR\n");
#endif
            delete old;
        } else if (rhs_->kind_ == ExpressionKind::CONSTANT && rhs_->const_val_ == 0) {
            Expression *old = lhs_;
            Copy(lhs_);
            old->Invalidate();
#ifdef EXPR_DEBUG
            printf("FOLDED OR/XOR\n");
#endif
            delete old;
        }
        break;
    default:
        break;
    }
}

void Expression::Tautology()
{
    if (lhs_ != nullptr) {
        lhs_->Tautology();
    }
    if (rhs_ != nullptr) {
        rhs_->Tautology();
    }

#ifdef EXPR_DEBUG
    printf("Tautology:\n");
    Print(stdout);
    printf("\n");
#endif

    if (kind_ != ExpressionKind::OR) {
        return;
    }

    assert(lhs_ != nullptr);
    assert(rhs_ != nullptr);

    if (lhs_->Equals(rhs_, ExpressionEqualityKind::INVERT)) {
        delete lhs_;
        delete rhs_;
        lhs_ = nullptr;
        rhs_ = nullptr;
        kind_ = ExpressionKind::CONSTANT;
        const_val_ = 1;
#ifdef EXPR_DEBUG
        printf("Reduced to 1\n");
#endif
    }
}

void Expression::Contradiction()
{
    if (lhs_ != nullptr) {
        lhs_->Contradiction();
    }
    if (rhs_ != nullptr) {
        rhs_->Contradiction();
    }

#ifdef EXPR_DEBUG
    printf("Contradiction:\n");
    Print(stdout);
    printf("\n");
#endif

    if (kind_ != ExpressionKind::AND) {
        return;
    }

    assert(lhs_ != nullptr);
    assert(rhs_ != nullptr);

    if (lhs_->Equals(rhs_, ExpressionEqualityKind::INVERT)) {
        delete lhs_;
        delete rhs_;
        lhs_ = nullptr;
        rhs_ = nullptr;
        kind_ = ExpressionKind::CONSTANT;
        const_val_ = 0;
#ifdef EXPR_DEBUG
        printf("Reduced to 0\n");
#endif
    }
}

void Expression::Associate()
{
    if (lhs_ != nullptr) {
        lhs_->Associate();
    }
    if (rhs_ != nullptr) {
        rhs_->Associate();
    }

#ifdef EXPR_DEBUG
    printf("Associate:\n");
    Print(stdout);
    printf("\n");
#endif

    if (kind_ != ExpressionKind::OR) {
        return;
    }

    assert (lhs_ != nullptr);
    assert (rhs_ != nullptr);

    if (lhs_->GetKind() != ExpressionKind::AND) {
        return;
    }

    if (rhs_->GetKind() != ExpressionKind::AND) {
        return;
    }

    Expression *lhs_lhs = lhs_->GetLhs();
    Expression *lhs_rhs = lhs_->GetRhs();
    Expression *rhs_lhs = rhs_->GetLhs();
    Expression *rhs_rhs = rhs_->GetRhs();

    assert (lhs_->lhs_ != nullptr);
    assert (lhs_->rhs_ != nullptr);
    assert (rhs_->lhs_ != nullptr);
    assert (rhs_->rhs_ != nullptr);

    struct assoc_row {
        Expression* a1;
        Expression* a2;
        Expression* b1;
        Expression* b2;
    };

    assoc_row arr[4] = {
        {lhs_lhs, lhs_rhs, rhs_lhs, rhs_rhs},
        {lhs_rhs, lhs_lhs, rhs_lhs, rhs_rhs},
        {lhs_lhs, lhs_rhs, rhs_rhs, rhs_lhs},
        {lhs_rhs, lhs_lhs, rhs_rhs, rhs_lhs}
    };

    for (int i = 0; i < 4; i++) {
        assoc_row r = arr[i];
        if (r.a1->Equals(r.b1, ExpressionEqualityKind::EQUAL)) {
            lhs_->Invalidate();
            delete lhs_;
            rhs_->Invalidate();
            delete rhs_;

            delete r.b1;

            lhs_ = r.a1;
            kind_ = ExpressionKind::AND;
            rhs_ = new Expression(ExpressionKind::OR, r.a2, r.b2);
#ifdef EXPR_DEBUG
            printf("Asociated:\n");
            Print(stdout);
            printf("\n");
#endif
            return;
        }
    }
}

bool Expression::Equals(Expression *e, ExpressionEqualityKind eq_kind)
{

    std::vector<Pin*> a_terms;
    std::vector<Pin*> b_terms;

    GetTerms(a_terms);
    e->GetTerms(b_terms);

    if (a_terms.size() != b_terms.size())
        return false;

    std::sort(a_terms.begin(), a_terms.end(), [](Pin*& a, Pin*& b) {
        return a->name_ > b->name_;
    });
    std::sort(b_terms.begin(), b_terms.end(), [](Pin*& a, Pin*& b) {
        return a->name_ > b->name_;
    });

    for (size_t i = 0; i < a_terms.size(); i++) {
        if (a_terms[i]->name_ != b_terms[i]->name_)
            return false;
    }

    size_t tbl_size = (1 << a_terms.size());
    for (size_t row = 0; row < tbl_size; row++) {

        std::vector<std::pair<Pin*, int>> vals;
        for (size_t i = 0; i < a_terms.size(); i++) {
            int v = (row >> i) & 0x1;
            Pin *p = a_terms[i];
            vals.push_back({p, v});
        }

        int a_res = Evaluate(vals);
        int b_res = e->Evaluate(vals);

        assert (a_res == 1 || a_res == 0);
        assert (b_res == 1 || b_res == 0);

        if (eq_kind == ExpressionEqualityKind::EQUAL) {
            if (a_res != b_res) {
                return false;
            }
        } else if (eq_kind == ExpressionEqualityKind::INVERT) {
            if (a_res == b_res) {
                return false;
            }
        } else {
            assert(false);
        }
    }

    return true;
}

void Expression::Substitute(Pin *pin, int val)
{
    if (lhs_ != nullptr) {
        lhs_->Substitute(pin, val);
    }
    if (rhs_ != nullptr) {
        rhs_->Substitute(pin, val);
    }

    if (kind_ != ExpressionKind::TERM) {
        return;
    }

    assert(pin_ != nullptr);

    if (pin_->name_ == pin->name_) {
        kind_ = ExpressionKind::CONSTANT;
        const_val_ = val;
    }
}

void Expression::GetTerms(std::vector<Pin*> &res)
{
    switch (kind_) {
    case ExpressionKind::TERM:
        assert(pin_ != nullptr);
        res.push_back(pin_);
        break;

    case ExpressionKind::AND:
    case ExpressionKind::OR:
    case ExpressionKind::XOR:
        assert(lhs_ != nullptr);
        assert(rhs_ != nullptr);
        lhs_->GetTerms(res);
        rhs_->GetTerms(res);
        break;

    case ExpressionKind::NOT:
        assert(lhs_ != nullptr);
        lhs_->GetTerms(res);
        break;

    default:
        break;
    }
}

int Expression::Evaluate(std::vector<std::pair<Pin*, int>> &terms)
{
    switch (kind_) {
    case ExpressionKind::CONSTANT:
        return const_val_;

    case ExpressionKind::AND:
    case ExpressionKind::OR:
    case ExpressionKind::XOR:
    {
        assert(lhs_ != nullptr);
        assert(rhs_ != nullptr);

        int lhs_v = lhs_->Evaluate(terms);
        int rhs_v = rhs_->Evaluate(terms);

        if (kind_ == ExpressionKind::AND) {
            return (lhs_v == 0x1 && rhs_v == 0x1) ? 1 : 0;
        } else if (kind_ == ExpressionKind::OR) {
            return (lhs_v == 0x1 || rhs_v == 0x1) ? 1 : 0;
        } else if (kind_ == ExpressionKind::XOR) {
            return ((lhs_v == 0x1 && rhs_v == 0x0) || (lhs_v == 0x0 && rhs_v == 0x1)) ? 1 : 0;
        }
        assert(false);
        break;
    }

    case ExpressionKind::NOT:
    {
        assert(lhs_ != nullptr);
        int lhs_v = lhs_->Evaluate(terms);
        return (lhs_v == 0x1) ? 0 : 1;
    }

    case ExpressionKind::TERM:
        for (auto & pr : terms) {
            if (pr.first->name_ == pin_->name_) {
                return pr.second;
            }
        }

        // Forces to have all terms defined otherwise we can't evaluate
        // an expression!
        assert(false);
    }

    assert(false);
    return 0;
}

void Expression::Simplify()
{
    ConstFold();
    DeMorgan();
    Associate();

    Tautology();
    ConstFold();

    Contradiction();
    ConstFold();
}

ExpressionKind Expression::GetKind()
{
    return kind_;
}

Expression* Expression::GetLhs()
{
    assert (lhs_ != nullptr);
    return lhs_;
}

Expression* Expression::GetRhs()
{
    assert (rhs_ != nullptr);
    return rhs_;
}

Pin* Expression::GetPin()
{
    assert (kind_ == ExpressionKind::TERM);
    assert (pin_ != nullptr);
    return pin_;
}

int Expression::GetConstValue()
{
    assert(kind_ == ExpressionKind::CONSTANT);
    return const_val_;
}

void Expression::Print(FILE *fd)
{
    char op = ' ';
    if (kind_ == ExpressionKind::AND) {
        op = and_op;
    } else if (kind_ == ExpressionKind::OR) {
        op = or_op;
    } else if (kind_ == ExpressionKind::XOR) {
        op = xor_op;
    }

    switch (kind_) {
    case ExpressionKind::TERM:
        assert(pin_ != nullptr);
        fprintf(fd, "%s", pin_->name_);
        break;

    case ExpressionKind::AND:
    case ExpressionKind::OR:
    case ExpressionKind::XOR:
        assert(lhs_ != nullptr);
        assert(rhs_ != nullptr);

        if (lhs_->kind_ != ExpressionKind::TERM && lhs_->kind_ != ExpressionKind::NOT) {
            fprintf(fd, "(");
        }
        lhs_->Print(fd);
        if (lhs_->kind_ != ExpressionKind::TERM && lhs_->kind_ != ExpressionKind::NOT) {
            fprintf(fd, ")");
        }
        fprintf(fd, " %c ", op);
        if (rhs_->kind_ != ExpressionKind::TERM && rhs_->kind_ != ExpressionKind::NOT) {
            fprintf(fd, "(");
        }
        rhs_->Print(fd);
        if (rhs_->kind_ != ExpressionKind::TERM && rhs_->kind_ != ExpressionKind::NOT) {
            fprintf(fd, ")");
        }
        break;

    case ExpressionKind::NOT:
        assert(lhs_ != nullptr);

        fprintf(fd, "!");
        if (lhs_->kind_ != ExpressionKind::TERM) {
            fprintf(fd, "(");
        }
        lhs_->Print(fd);
        if (lhs_->kind_ != ExpressionKind::TERM) {
            fprintf(fd, ")");
        }
        break;

    case ExpressionKind::CONSTANT:
        fprintf(fd, "%d", const_val_);
        break;
    }
}

}