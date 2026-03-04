
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

        void AddSimulation(Simulation *simulation);
        std::vector<Simulation*>& GetSimulations();

    private:
        UnateKind GetUnateness();

        void WriteTable(FILE *f, size_t tab, std::vector<std::vector<double>>& data,
                        std::string title);

        Pin* pin_;
        Template *template_;

        std::vector<std::vector<NanoSecond>> rise_delays_;
        std::vector<std::vector<NanoSecond>> rise_transitions_;
        std::vector<std::vector<PicoJoule>>  rise_power_;

        std::vector<std::vector<NanoSecond>> fall_delays_;
        std::vector<std::vector<NanoSecond>> fall_transitions_;
        std::vector<std::vector<PicoJoule>>  fall_power_;

        // Input and output pin states
        int64_t in_a_;
        int64_t in_b_;
        int out_a_;
        int out_b_;

        ArcKind kind_;

        std::vector<Simulation*> simulations_;
};

}

#endif