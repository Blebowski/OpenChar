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

#ifndef PIN_H
#define PIN_H

#include <string>
#include <vector>
#include <ranges>

#include "open_char.h"

#include "Arc.h"
#include "Expression.h"
#include "Simulation.h"

namespace open_char {

class Pin {

    public:
        Cell *cell_;

        const std::string name_;
        const PinDir direction_;
        const PinKind kind_;

        Pin();
        ~Pin();
        Pin(Cell *cell, std::string name, PinDir direction, PinKind kind);

        void AddLogicTableEntry(int64_t inputs, int output);
        std::pair<int64_t, int> GetLogicTableEntry(int index);

        // TODO: Avoid copying here
        void AddArc(Arc arc);
        std::vector<Arc>& GetArcs();

        const std::vector<std::pair<int64_t, int>>& GetLogicTable();
        void PrintLogicTable();

        void SetLogicFunction(Expression *e);
        Expression *GetLogicFunction();
        void PrintLogicFunction();

        void SetPolarity(int polarity);
        int  GetPolarity();

        void SetCapacitanceRise(PicoFarad min, PicoFarad max, PicoFarad avg);
        void SetCapacitanceFall(PicoFarad min, PicoFarad max, PicoFarad avg);

        PicoFarad GetCapacitanceRiseMax();
        PicoFarad GetCapacitanceRiseMin();
        PicoFarad GetCapacitanceRiseAvg();

        PicoFarad GetCapacitanceFallMax();
        PicoFarad GetCapacitanceFallMin();
        PicoFarad GetCapacitanceFallAvg();

        void AddSimulation(Simulation *simulation);
        std::vector<Simulation*>& GetSimulations();

        auto GetSimulations(SimClass sim_class) {
            return std::views::filter(simulations_,
                [sim_class](const Simulation *sim) {
                    return sim->class_ == sim_class;
                }
            );
        };

        void WriteLiberty(FILE *f, size_t tab);
        void WriteVerilog(FILE *f, size_t tab);

    private:
        // First value - Cell inputs
        // Second value - Cell output
        std::vector<std::pair<int64_t, int>> logic_table_;

        std::vector<Arc> arcs_;

        Expression *func_;

        int polarity_;

        std::vector<Simulation*> simulations_;

        PicoFarad cap_rise_min_;
        PicoFarad cap_rise_max_;
        PicoFarad cap_rise_avg_;
        PicoFarad cap_fall_min_;
        PicoFarad cap_fall_max_;
        PicoFarad cap_fall_avg_;
};

}

#endif