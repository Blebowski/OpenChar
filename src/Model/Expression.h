
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>

#include "open_char.h"

namespace open_char {

class Expression {

    public:
        Expression(ExprKind kind, Expression *lhs, Expression *rhs);
        Expression(ExprKind kind, Expression *lhs);
        Expression(ExprKind kind, Pin *pin);
        Expression(ExprKind kind, int const_val);
        ~Expression();

        void Simplify();
        bool Equals(Expression *e, ExprEqualKind eq_kind);
        void Substitute(Pin *pin, int val);
        void Print(FILE *fd);

        ExprKind GetKind();
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

        ExprKind kind_;
        Expression *lhs_;
        Expression *rhs_;
        int const_val_;
        Pin *pin_;
};

}

#endif