
#ifndef LOGIC_TABLE_H
#define LOGIC_TABLE_H

#include <vector>
#include <cinttypes>

#include "open_char.h"

namespace open_char {

class LogicTable {

    public:
        std::vector<open_char::Pin*> input_pins;

        // Bit 0        Result
        // Bit 1 - 63   Input pin values
        std::vector<int64_t> table;

        LogicTable();
};

}

#endif