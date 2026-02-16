
#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>

#include "open_char.h"

#include "Expression.h"
#include "Utils.h"

namespace open_char {

class Algorithms {

    public:
        Algorithms(Context *ctx);

        void MeasureLogicTableAndLeakage(Cell &cell);
        void MeasureComboDelay(Cell &cell);
        void CalculateLogicFunctions(Cell &cell);

        void CharacterizeCells(Cell &cell);

    private:
        Context *ctx_;
        int ToLogic(Volt val);
        int GetBit(int64_t v, size_t index);
        NanoWatt ComputePower(MicroAmp i, Volt v);
        NanoSecond FindVoltage(Waves &w, Pin *pin, int from, Volt v);
        Expression* SumOfProducts(Cell& cell, Pin& opin);
        Expression* ProductOfSums(Cell& cell, Pin& opin);
        Expression* RecognizeXor(Cell& cell, Pin& opin);
        int MeasureOneStateDelay(Pin *opin, int64_t in_from, int64_t in_to,
                                 int out_from, int out_to);
};

}

#endif