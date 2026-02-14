

#include <cassert>

#include "Expression.h"
#include "Utils.h"
#include "Pin.h"

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

void Expression::ConstFold()
{
    switch (kind_) {
    case ExpressionKind::AND:
        assert(lhs_ != nullptr);
        assert(rhs_ != nullptr);
        if (lhs_->kind_ == ExpressionKind::CONSTANT && lhs_->const_val_ == 1) {
            Expression *old = rhs_;
            Copy(rhs_);
            old->Invalidate();
            delete old;
        } else if (rhs_->kind_ == ExpressionKind::CONSTANT && rhs_->const_val_ == 1) {
            Expression *old = lhs_;
            Copy(lhs_);
            old->Invalidate();
            delete old;
        }
        break;
    case ExpressionKind::OR:
        assert(lhs_ != nullptr);
        assert(rhs_ != nullptr);
        if (lhs_->kind_ == ExpressionKind::CONSTANT && lhs_->const_val_ == 0) {
            Expression *old = rhs_;
            Copy(rhs_);
            old->Invalidate();
            delete old;
        } else if (rhs_->kind_ == ExpressionKind::CONSTANT && rhs_->const_val_ == 0) {
            Expression *old = lhs_;
            Copy(lhs_);
            old->Invalidate();
            delete old;
        }
        break;
    default:
        break;
    }
}

void Expression::Simplify()
{
    if (lhs_ != nullptr)
        lhs_->Simplify();
    if (rhs_ != nullptr)
        rhs_->Simplify();

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