
#ifndef PIN_H
#define PIN_H

#include <string>
#include <vector>

#include "open_char.h"
#include "DelayTable.h"

namespace open_char {

class Pin {

    public:
        Cell *cell_;

        const std::string name_;
        const PinDirection direction_;
        const PinKind kind_;

        Pin();
        Pin(Cell *cell, std::string name, PinDirection direction, PinKind kind);

        void AddLogicTableEntry(int64_t inputs, int output);
        std::pair<int64_t, int> GetLogicTableEntry(int index);

        // TODO: Avoid copying here
        void AddDelayTable(DelayTable delay_table);

        const std::vector<std::pair<int64_t, int>>& GetLogicTable();
        void PrintLogicTable();

        void WriteLiberty(FILE *f, size_t tab);

    private:
        // First value - Cell inputs
        // Second value - Cell output
        std::vector<std::pair<int64_t, int>> logic_table_;

        std::vector<DelayTable> delay_tables_;

};

}

#endif