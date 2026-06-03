
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

        void PrepareSanitySim(Cell &cell);
        bool CheckSanitySim(Cell &cell);

        void PrepareInputCapSims(Cell &cell);
        bool MeasureInputCap(Cell &cell);

        void PrepareComboLogicTableAndLeakageSims(Cell &cell);
        bool MeasureComboLogicTables(Cell &cell);
        bool MeasureComboLeakage(Cell &cell);
        void CalculateComboLogicFunctions(Cell &cell);

        void PrepareComboDelayAndPowerSims(Cell &cell);
        bool MeasureComboDelays(Cell &cell);
        bool MeasureComboTransitions(Cell &cell);
        bool MeasureComboPowers(Cell &cell);

        void PrepareSeqAsyncFunctionSims(Cell &cell);
        bool MeasureSeqAsyncFunctions(Cell &cell);

        void PrepareSeqCellKindSims(Cell &cell);
        bool MeasureSeqCellKind(Cell &cell);

        void PrepareFFClockPolaritySims(Cell &cell);
        bool MeasureFFClockPolarity(Cell &cell);

        void PrepareFFClockDelaySims(Cell &cell);
        bool MeasureFFClockDelay(Cell &cell);
        bool MeasureFFClockTransition(Cell &cell);
        bool MeasureFFClockPowers(Cell &cell);

        void PrepareSetupSims(Cell &cell);
        void PrepareOneSetupSim(Cell &cell, size_t arc_index,
                                size_t d_tran_index, NanoSecond d_tran,
                                size_t ck_tran_index, NanoSecond ck_tran,
                                NanoSecond ck_q_base, NanoSecond ck_d_skew,
                                NanoSecond step);
        bool MeasureSetup(Cell &cell);

        bool CharacterizeLibrary();

    private:
        Context *ctx_;
        int ToLogic(Volt val);
        Simulation* NewSimulation(std::string name, SimulationKind kind, Cell *cell);
        int GetBit(int64_t v, size_t index);
        Expression* ComboSumOfProducts(Cell& cell, Pin& o_pin);
        Expression* ComboProductOfSums(Cell& cell, Pin& o_pin);
        Expression* ComboRecognizeXor(Cell& cell, Pin& o_pin);

        int PrepareOneComboArcSims(Pin &o_pin, int64_t in_from, int64_t in_to,
                                   int out_from, int out_to);
};

}

#endif