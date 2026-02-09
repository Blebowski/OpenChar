
#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "open_char.h"
#include "Context.h"

namespace open_char {

class Algorithms {

    public:
        Algorithms(Context *ctx);

        bool MeasureLogicFunction(Cell &cell);
        bool MeasureComboDelay(Cell &cell);

    private:
        Context *ctx_;
        int ToLogic(Volt val);
        int GetBit(int64_t v, size_t index);
        NanoSecond FindEdge(Waves &w, Pin *pin, int from);
        int MeasureOneStateDelay(Pin *opin, int64_t in_from, int64_t in_to,
                                 int out_from, int out_to);
};

}

#endif