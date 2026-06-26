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