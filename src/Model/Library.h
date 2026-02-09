
#ifndef LIBRARY_H
#define LIBRARY_H

#include "Cell.h"
#include "OpCond.h"
#include "Template.h"

namespace open_char {

class Library {

    public:
        Library();

        std::pair<Cell&, bool> AddCell(std::string name);
        bool HasCell(std::string name);
        Cell &GetCell(std::string name);
        std::map<std::string, Cell>& GetCells();

        void SetDefaultSupplyVdd(std::string name, double val);
        void SetDefaultSupplyVdd(double val);
        void SetDefaultSupplyGnd(std::string name, double val);

        bool HasSupply(std::string vdd_name);
        Supply& GetSupply(std::string vdd_name);

        std::pair<Template&, bool> AddTemplate(std::string name);
        bool HasTemplate(std::string name);
        Template &GetTemplate(std::string name);

        OpCond& GetOpCond();

        void WriteLiberty(const std::string &name);

    private:
        std::map<std::string, Cell> cells_;
        std::vector<Supply> supplies_;
        OpCond op_cond_;
        std::map<std::string, Template> templates_;

};

}

#endif