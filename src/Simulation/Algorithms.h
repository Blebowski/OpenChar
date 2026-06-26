////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>

#include "open_char.h"

#include "Expression.h"
#include "Arc.h"
#include "Utils.h"

namespace open_char {

class Algorithms {

    public:
        Algorithms(Context *ctx);

        void PrepareSanitySim(Cell &cell);
        bool CheckSanitySim(Cell &cell);

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Algorithms for all cells
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrepareInputCapSims(Cell &cell);
        bool MeasureInputCap(Cell &cell);

        void PrepareLeakageSims(Cell &cell);
        bool MeasureLeakage(Cell &cell);

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Algorithms for combinatorial cells
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrepareComLogicTablesSims(Cell &cell);
        bool MeasureComLogicTables(Cell &cell);
        void CalculateComLogicFunctions(Cell &cell);

        void PrepareComDelayTransitionPowerSims(Cell &cell);
        bool MeasureComDelaysTransitionsPowers(Cell &cell);

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Algorithms for arbitrary sequential cells
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrepareSeqAsyncFunctionSims(Cell &cell);
        bool MeasureSeqAsyncFunctions(Cell &cell);

        void PrepareSeqEdgeOrLvlSims(Cell &cell);
        bool MeasureSeqEdgeOrLvl(Cell &cell);

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Algorithms for Flip-flops
        ///////////////////////////////////////////////////////////////////////////////////////////
        void PrepareFFClockPolaritySims(Cell &cell);
        bool MeasureFFClockPolarity(Cell &cell);

        void PrepareFFClockDelayTransitionPowerSims(Cell &cell);
        bool MeasureFFClockDelaysTransitionsPowers(Cell &cell);

        void PrepareFFSetupOrHoldSims(Cell &cell, ArcKind a_kind);
        std::pair<bool,bool> MeasureFFSetupOrHold(Cell &cell, ArcKind a_kind);

        void PrepareFFClockMPWSims(Cell &cell);
        std::pair<bool,bool> MeasureFFClockMPW(Cell &cell);

        bool CharacterizeLibrary();

    private:
        Context *ctx_;
        int ToLogic(Volt val, Volt vdd);
        Simulation* NewSimulation(SimClass sim_class, std::string name, SimKind kind, Cell *cell);
        int GetBit(int64_t v, size_t index);
        Expression* ComboSumOfProducts(Cell& cell, Pin& o_pin);
        Expression* ComboProductOfSums(Cell& cell, Pin& o_pin);
        Expression* ComboRecognizeXor(Cell& cell, Pin& o_pin);

        int PrepareOneComboArcSims(Pin &o_pin, int64_t in_from, int64_t in_to,
                                   int out_from, int out_to);
        void MeasureOneComboDelay(Simulation *sim, Waves &w, Cell &cell, Pin &o_pin, Arc &arc,
                                  size_t i_tran_index, size_t o_cap_index);
        void MeasureOneComboTransition(Simulation *sim, Waves &w, Cell &cell, Pin &o_pin, Arc &arc,
                                       size_t i_tran_index, size_t o_cap_index);
        void MeasureOneComboPower(Simulation *sim, Waves &w, Cell &cell, Pin &o_pin, Arc &arc,
                                  size_t i_tran_index, size_t o_cap_index);

        void MeasureOneFFClockDelay(Cell &cell, Simulation *sim, Waves &w, Arc &arc,
                                    Pin &o_pin, size_t i_tran_index, size_t o_cap_index);
        void MeasureOneFFClockTransition(Simulation *sim, Waves &w, Cell &cell, Arc &arc,
                                         Pin &o_pin, size_t i_tran_index, size_t o_cap_index);
        void MeasureOneFFClockPower(Cell &cell, Simulation *sim, Waves &w, Arc &arc,
                                    Pin &o_pin, size_t i_tran_index, size_t o_cap_index);

        void PrepareOneFFSetupOrHoldSim(Cell &cell, ArcKind a_kind, size_t arc_index,
                                        size_t d_tran_index, NanoSecond d_tran,
                                        size_t ck_tran_index, NanoSecond ck_tran,
                                        NanoSecond ck_q_base, NanoSecond ck_d_skew,
                                        NanoSecond step);
        void PrepareOneFFClockMPWSim(Cell &cell, size_t arc_index, NanoSecond ck_q_base,
                                     NanoSecond pulse_width, NanoSecond step, bool high);

};

}

#endif