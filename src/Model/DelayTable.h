
#ifndef DELAY_TABLE_H
#define DELAY_TABLE_H

#include <vector>

#include "open_char.h"
#include "Utils.h"

namespace open_char {

class DelayTable {

    public:
        DelayTable();
        void Print();

        Pin *pin_;

        int64_t in_from_;
        int64_t in_to_;
        int out_from_;
        int out_to_;

        std::vector<std::vector<NanoSecond>> delay_;
};

}

#endif