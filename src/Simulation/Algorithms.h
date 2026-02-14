
#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "open_char.h"

#include "Utils.h"

namespace open_char {

class Algorithms {

    public:
        Algorithms(Context *ctx);

        void MeasureLogicFunction(Cell &cell);
        void MeasureComboDelay(Cell &cell);
        void CalculateLogicFunctions(Cell &cell);

        void CharacterizeCells(Cell &cell);

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