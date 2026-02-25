
#ifndef FLIP_FLOP_H
#define FLIP_FLOP_H

#include "open_char.h"

namespace open_char {

class FlipFlop {

    public:
        FlipFlop(Cell *cell);
        ~FlipFlop();

        void SetClear(Expression *e);
        void SetPreset(Expression *e);
        void SetAsyncPriority(AsyncPriority async_priority);
        void SetNextState(Expression *e);

        void SetClockPin(Pin *pin);
        Pin* GetClockPin();

        void WriteLiberty(FILE *f, size_t tab);

    private:
        Cell *cell_;
        Pin *clock_pin_;

        Expression *set_;
        Expression *clr_;
        Expression *next_state_;

        AsyncPriority async_priority_;
};

}

#endif