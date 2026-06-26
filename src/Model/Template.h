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

#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <vector>
#include <string>

#include "open_char.h"

namespace open_char {

class Template {

    public:
        Template(std::string name);

        void WriteLiberty(size_t tab, FILE* f);

        void SetKind(TemplKind kind);
        TemplKind GetKind();

        std::string GetName();

        std::vector<double>& GetIndex1();
        std::vector<double>& GetIndex2();

        void AddIndex1(double val);
        void AddIndex2(double val);

    private:
        const std::string name_;
        TemplKind kind_;
        std::vector<double> index_1_;
        std::vector<double> index_2_;

};

}

#endif