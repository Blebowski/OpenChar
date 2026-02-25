
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>

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
        bool Equals(Expression *e, ExpressionEqualityKind eq_kind);
        void Substitute(Pin *pin, int val);
        void Print(FILE *fd);

        ExpressionKind GetKind();
        Expression *GetLhs();
        Expression *GetRhs();
        Pin *GetPin();
        int GetConstValue();

    private:
        void Copy(Expression *e);
        void ConstFold();
        void Associate();
        void DeMorgan();
        void Tautology();
        void Contradiction();
        void Invalidate();

        void GetTerms(std::vector<Pin*> &res);
        int Evaluate(std::vector<std::pair<Pin*, int>> &terms);

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