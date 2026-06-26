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

#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include "open_char.h"

namespace open_char {

class Sequential {

    public:
        Sequential(Cell *cell);
        ~Sequential();

        void SetClear(Expression *e);
        void SetPreset(Expression *e);
        void SetAsyncPriority(AsyncPrio async_priority);
        void SetNextState(Expression *e);

        void SetClockPolarity(EdgeKind clock_polarity);
        EdgeKind GetClockPolarity();

        void SetEnablePolarity(int polarity);
        int GetEnablePolarity();

        void SetKind(SeqKind kind);
        SeqKind GetKind();

        void SetClockPin(Pin *pin);
        Pin* GetClockPin();

        void WriteLiberty(FILE *f, size_t tab);

    private:
        Cell *cell_;
        Pin *clock_pin_;
        SeqKind kind_;
        EdgeKind clock_polarity_;
        int enable_polarity_;

        Expression *set_;
        Expression *clr_;
        Expression *next_state_;

        AsyncPrio async_priority_;
};

}

#endif