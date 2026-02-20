
#ifndef ARC_H
#define ARC_H

#include <vector>

#include "open_char.h"

namespace open_char {

class Arc {

    public:
        Arc(Pin *pin, Template *templ, int64_t in_a, int64_t in_b, int out_a, int out_b);

        void Print();
        void WriteLiberty(FILE *f, size_t tab);

        void AddRiseDelay(size_t row, NanoSecond delay);
        std::vector<std::vector<NanoSecond>>& GetRiseDelays();

        void AddFallDelay(size_t row, NanoSecond delay);
        std::vector<std::vector<NanoSecond>>& GetFallDelays();

        void AddRiseTransition(size_t row, NanoSecond transition);
        std::vector<std::vector<NanoSecond>>& GetRiseTransitions();

        void AddFallTransition(size_t row, NanoSecond transition);
        std::vector<std::vector<NanoSecond>>& GetFallTransitions();

        void AddRisePower(size_t row, PicoJoule energy);
        std::vector<std::vector<PicoJoule>>& GetRisePowers();

        void AddFallPower(size_t row, PicoJoule energy);
        std::vector<std::vector<NanoSecond>>& GetFallPowers();

    private:
        Pin* GetRelatedPin();
        bool isPositiveUnate();

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
};

}

#endif