
#ifndef PIN_H
#define PIN_H

#include <string>
#include <vector>

#include "open_char.h"

namespace open_char {

class Pin {

    public:
        Cell *cell_;
        const std::string name_;
        const PinDirection direction_;
        const PinKind kind_;


        // Bit 63    - Result
        // Bits 62-0 - Inputs in the order of input pins of the above cell
        std::vector<int64_t> logic_table_;

        Pin(Cell *cell, std::string name, PinDirection direction, PinKind kind);

        void AddLogTableEntry(int64_t inputs, int output);
        void PrintLogicTable();
};

}

#endif