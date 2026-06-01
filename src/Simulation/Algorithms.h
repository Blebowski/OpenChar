
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

        void PrepareInputCapSims(Cell &cell);
        void MeasureInputCap(Cell &cell);

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

        void PrepareFFClockDelaySims(Cell &cell);
        void MeasureFFClockDelay(Cell &cell);
        void MeasureFFClockTransition(Cell &cell);
        void MeasureFFClockPowers(Cell &cell);

        void PrepareSetupSims(Cell &cell);
        void PrepareOneSetupSim(Cell &cell, size_t arc_index,
                                size_t d_tran_index, NanoSecond d_tran,
                                size_t ck_tran_index, NanoSecond ck_tran,
                                NanoSecond ck_q_base, NanoSecond ck_d_skew,
                                NanoSecond step);
        bool MeasureSetup(Cell &cell);

        void CharacterizeLibrary();

    private:
        Context *ctx_;
        int ToLogic(Volt val);
        Simulation* NewSimulation(std::string name, SimulationKind kind, Cell *cell);
        int GetBit(int64_t v, size_t index);
        Expression* ComboSumOfProducts(Cell& cell, Pin& o_pin);
        Expression* ComboProductOfSums(Cell& cell, Pin& o_pin);
        Expression* ComboRecognizeXor(Cell& cell, Pin& o_pin);

        int PrepareComboArcSims(Pin &o_pin, int64_t in_from, int64_t in_to,
                                int out_from, int out_to);
};

}

#endif