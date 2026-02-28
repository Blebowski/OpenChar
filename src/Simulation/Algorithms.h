
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

        void PrepareComboLogicTableAndLeakageSims(Cell &cell);
        void MeasureComboLogicTables(Cell &cell);
        void MeasureComboLeakage(Cell &cell);
        void CalculateComboLogicFunctions(Cell &cell);

        void PrepareComboDelayAndPowerSims(Cell &cell);
        void MeasureComboDelays(Cell &cell);
        void MeasureComboTransitions(Cell &cell);
        void MeasureComboPowers(Cell &cell);

        void PrepareSeqAsyncFunctionSims(Cell &cell);
        void MeasureSeqAsyncFunctions(Cell &cell);

        void PrepareSeqCellKindSims(Cell &cell);
        void MeasureSeqCellKind(Cell &cell);

        void PrepareFFClockPolaritySims(Cell &cell);
        void MeasureFFClockPolarity(Cell &cell);

        void PrepareSeqClockDelaySims(Cell &cell);
        void MeasureSeqClockDelay(Cell &cell);
        void MeasureSeqClockTransition(Cell &cell);
        void MeasureSeqClockPowers(Cell &cell);

        void CharacterizeLibrary();

    private:
        Context *ctx_;
        int ToLogic(Volt val);
        int GetBit(int64_t v, size_t index);
        Expression* ComboSumOfProducts(Cell& cell, Pin& o_pin);
        Expression* ComboProductOfSums(Cell& cell, Pin& o_pin);
        Expression* ComboRecognizeXor(Cell& cell, Pin& o_pin);

        int PrepareComboArcSims(Pin &o_pin, int64_t in_from, int64_t in_to,
                                int out_from, int out_to);
};

}

#endif