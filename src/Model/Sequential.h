
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