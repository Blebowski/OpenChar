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

#ifndef ARC_H
#define ARC_H

#include <vector>

#include "open_char.h"

namespace open_char {

class Arc {

    public:
        Arc(Pin *pin, Template *templ, ArcKind kind,
            int64_t in_a, int64_t in_b, int out_a, int out_b);

        void Print();
        void WriteLiberty(FILE *f, size_t tab);

        Pin* GetRelatedPin();
        Template* GetTemplate();

        ArcKind GetKind();

        void SetRiseDelay(size_t row, size_t col, NanoSecond delay);
        std::vector<std::vector<NanoSecond>>& GetRiseDelays();

        void SetFallDelay(size_t row, size_t col, NanoSecond delay);
        std::vector<std::vector<NanoSecond>>& GetFallDelays();

        void SetRiseTransition(size_t row, size_t col, NanoSecond transition);
        std::vector<std::vector<NanoSecond>>& GetRiseTransitions();

        void SetFallTransition(size_t row, size_t col, NanoSecond transition);
        std::vector<std::vector<NanoSecond>>& GetFallTransitions();

        void SetRisePower(size_t row, size_t col, PicoJoule energy);
        std::vector<std::vector<PicoJoule>>& GetRisePowers();

        void SetFallPower(size_t row, size_t col, PicoJoule energy);
        std::vector<std::vector<NanoSecond>>& GetFallPowers();

        void SetRiseConstraint(size_t row, size_t col, NanoSecond constr);
        std::vector<std::vector<NanoSecond>>& GetRiseConstraints();

        void SetFallConstraint(size_t row, size_t col, NanoSecond constr);
        std::vector<std::vector<NanoSecond>>& GetFallConstraints();

        void AddSimulation(size_t row, size_t col, Simulation *simulation);
        std::vector<std::vector<std::vector<Simulation*>>>& GetSimulations();

    private:
        UnateKind GetUnateness();

        void WriteTable(FILE *f, size_t tab, std::vector<std::vector<double>>& data,
                        std::string title);

        Pin* pin_;
        Template *template_;

        std::vector<std::vector<NanoSecond>> rise_delays_;
        std::vector<std::vector<NanoSecond>> rise_transitions_;
        std::vector<std::vector<PicoJoule>>  rise_powers_;
        std::vector<std::vector<NanoSecond>> rise_constraints_;

        std::vector<std::vector<NanoSecond>> fall_delays_;
        std::vector<std::vector<NanoSecond>> fall_transitions_;
        std::vector<std::vector<PicoJoule>>  fall_powers_;
        std::vector<std::vector<NanoSecond>> fall_constraints_;

        // Input and output pin states
        int64_t in_a_;
        int64_t in_b_;
        int out_a_;
        int out_b_;

        ArcKind kind_;

        std::vector<std::vector<std::vector<Simulation*>>> simulations_;
};

}

#endif