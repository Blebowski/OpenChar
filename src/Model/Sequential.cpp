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

#include <cassert>

#include "Sequential.h"
#include "Expression.h"
#include "Utils.h"
#include "Pin.h"

namespace open_char {

Sequential::Sequential(Cell *cell) :
    cell_(cell),
    clock_pin_(nullptr),
    set_(nullptr),
    clr_(nullptr),
    next_state_(nullptr)
{};

Sequential::~Sequential()
{
    if (set_ != nullptr) {
        delete set_;
    }
    if (clr_ != nullptr) {
        delete clr_;
    }
    if (next_state_ != nullptr) {
        delete next_state_;
    }
}

void Sequential::SetClear(Expression *e)
{
    clr_ = e;
}

void Sequential::SetPreset(Expression *e)
{
    set_ = e;
}

void Sequential::SetNextState(Expression *e)
{
    next_state_ = e;
}

void Sequential::SetClockPin(Pin *pin)
{
    clock_pin_ = pin;
}

Pin* Sequential::GetClockPin()
{
    assert(clock_pin_ != nullptr);
    return clock_pin_;
}

void Sequential::SetAsyncPriority(AsyncPrio async_priority)
{
    async_priority_ = async_priority;
}

void Sequential::SetKind(SeqKind kind)
{
    kind_ = kind;
}

SeqKind Sequential::GetKind()
{
    return kind_;
}

void Sequential::SetClockPolarity(EdgeKind clock_polarity)
{
    clock_polarity_ = clock_polarity;
}

EdgeKind Sequential::GetClockPolarity()
{
    return clock_polarity_;
}

void Sequential::SetEnablePolarity(int enable_polarity)
{
    enable_polarity_ = enable_polarity;
}

int Sequential::GetEnablePolarity()
{
    return enable_polarity_;
}

void Sequential::WriteLiberty(FILE *f, size_t tab)
{
    if (kind_ == SeqKind::FLIP_FLOP) {
        TAB_FPRINTF(tab, f, "ff(IQ, IQN) {\n");
    } else {
        TAB_FPRINTF(tab, f, "latch(IQ, IQN) {\n");
    }
    tab++;

    TAB_FPRINTF(tab, f, "next_state: TODO\n");
    TAB_FPRINTF(tab, f, "clocked_on: %s\n", clock_pin_->name_);

    if (clr_ != nullptr) {
        TAB_FPRINTF(tab, f, "clear : \"");
        clr_->Print(f);
        fprintf(f, "\"\n");
    }

    if (set_ != nullptr) {
        TAB_FPRINTF(tab, f, "preset : \"");
        set_->Print(f);
        fprintf(f, "\"\n");
    }

    tab--;
    TAB_FPRINTF(tab, f, "}\n");
}

}

