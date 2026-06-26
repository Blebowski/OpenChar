////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LIBRARY_H
#define LIBRARY_H

#include "Cell.h"
#include "OpCond.h"
#include "Template.h"

namespace open_char {

class Library {

    public:
        Library(Context *ctx);

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
        void WriteVerilog(const std::string &name);

    private:
        std::map<std::string, Cell> cells_;
        std::vector<Supply> supplies_;
        OpCond op_cond_;
        std::map<std::string, Template> templates_;
        Context *ctx_;

};

}

#endif