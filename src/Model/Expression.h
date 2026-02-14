
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "open_char.h"

namespace open_char {

class Expression {

    public:
        Expression(ExpressionKind kind, Expression *lhs, Expression *rhs);
        Expression(ExpressionKind kind, Expression *lhs);
        Expression(ExpressionKind kind, Pin *pin);
        Expression(ExpressionKind kind, int const_val);
        ~Expression();

        void Simplify();
        void Print();

        ExpressionKind GetKind();
        Expression *GetLhs();
        Expression *GetRhs();
        Pin *GetPin();
        int GetConstValue();

    private:
        void Copy(Expression *e);
        void ConstFold();
        void Invalidate();

        const char and_op = '&';
        const char or_op = '|';
        const char xor_op = '^';

        ExpressionKind kind_;
        Expression *lhs_;
        Expression *rhs_;
        int const_val_;
        Pin *pin_;
};

}

#endif