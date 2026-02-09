
#ifndef DELAY_TABLE_H
#define DELAY_TABLE_H

#include <vector>

#include "open_char.h"

namespace open_char {

class DelayTable {

    public:
        DelayTable();
        std::pair<Pin *,EdgeKind> GetRelatedPin();
        void Print();
        void WriteLiberty(FILE *f, size_t tab);

        Pin *pin_;
        Template *template_;

        int64_t in_from_;
        int64_t in_to_;
        int out_from_;
        int out_to_;

        std::vector<std::vector<NanoSecond>> delay_;
};

}

#endif