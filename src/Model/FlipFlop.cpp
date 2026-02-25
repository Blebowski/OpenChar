
#include <cassert>

#include "FlipFlop.h"
#include "Expression.h"
#include "Utils.h"
#include "Pin.h"

namespace open_char {

FlipFlop::FlipFlop(Cell *cell) :
    cell_(cell),
    clock_pin_(nullptr),
    set_(nullptr),
    clr_(nullptr),
    next_state_(nullptr)
{};

FlipFlop::~FlipFlop()
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

void FlipFlop::SetClear(Expression *e)
{
    clr_ = e;
}

void FlipFlop::SetPreset(Expression *e)
{
    set_ = e;
}

void FlipFlop::SetNextState(Expression *e)
{
    next_state_ = e;
}

void FlipFlop::SetClockPin(Pin *pin)
{
    clock_pin_ = pin;
}

Pin* FlipFlop::GetClockPin()
{
    assert(clock_pin_ != nullptr);
    return clock_pin_;
}

void FlipFlop::SetAsyncPriority(AsyncPriority async_priority)
{
    async_priority_ = async_priority;
}

void FlipFlop::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "ff(IQ, IQN) {\n");
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

