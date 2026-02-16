
#ifndef DELAY_TABLE_H
#define DELAY_TABLE_H

#include <vector>

#include "open_char.h"

namespace open_char {

class DelayTable {

    public:
        DelayTable(Pin *pin, Template *templ, int64_t in_from,
                   int64_t in_to, int out_from, int out_to);

        void Print();
        void WriteLiberty(FILE *f, size_t tab);

        void AddDelay(size_t row, NanoSecond delay);
        std::vector<std::vector<NanoSecond>>& GetDelays();

        void AddTransition(size_t row, NanoSecond transition);
        std::vector<std::vector<NanoSecond>>& GetTransitions();

    private:
        std::pair<Pin *,EdgeKind> GetRelatedPin();

        Pin *pin_;
        Template *template_;

        std::vector<std::vector<NanoSecond>> delays_;
        std::vector<std::vector<NanoSecond>> transitions_;

        int64_t in_from_;
        int64_t in_to_;
        int out_from_;
        int out_to_;
};

}

#endif