
#ifndef PIN_H
#define PIN_H

#include <string>
#include <vector>

#include "open_char.h"
#include "Arc.h"
#include "Expression.h"

namespace open_char {

class Pin {

    public:
        Cell *cell_;

        const std::string name_;
        const PinDirection direction_;
        const PinKind kind_;

        Pin();
        ~Pin();
        Pin(Cell *cell, std::string name, PinDirection direction, PinKind kind);

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

        void WriteLiberty(FILE *f, size_t tab);

    private:
        // First value - Cell inputs
        // Second value - Cell output
        std::vector<std::pair<int64_t, int>> logic_table_;

        std::vector<Arc> arcs_;

        Expression *func_;

};

}

#endif