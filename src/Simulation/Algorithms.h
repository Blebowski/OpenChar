
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

        void PrepareLogicTableAndLeakageSims(Cell &cell);
        void PrepareComboDelaySims(Cell &cell);

        void CalculateLogicFunctions(Cell &cell);

        void CharacterizeLibrary();

    private:
        Context *ctx_;
        int ToLogic(Volt val);
        int GetBit(int64_t v, size_t index);
        Expression* SumOfProducts(Cell& cell, Pin& opin);
        Expression* ProductOfSums(Cell& cell, Pin& opin);
        Expression* RecognizeXor(Cell& cell, Pin& opin);
        int PrepareComboArcSims(Pin *opin, int64_t in_from, int64_t in_to,
                                int out_from, int out_to);
};

}

#endif