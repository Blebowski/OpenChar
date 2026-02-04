
#ifndef LIBRARY_H
#define LIBRARY_H

#include "Cell.h"
#include "OpCond.h"

namespace open_char {

class Library {

    public:
        Library();

        std::pair<Cell&, bool> AddCell(std::string name);
        bool HasCell(std::string name);
        Cell &GetCell(std::string name);

        void SetDefaultSupplyVdd(std::string name, double val);
        void SetDefaultSupplyVdd(double val);
        void SetDefaultSupplyGnd(std::string name, double val);

        bool HasSupply(std::string vdd_name);
        Supply& GetSupply(std::string vdd_name);

        OpCond& GetOpCond();

    private:
        std::map<std::string, Cell> cells_;
        std::vector<Supply> supplies_;
        OpCond op_cond_;

};

}

#endif