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

#include "Template.h"
#include "Utils.h"

namespace open_char {

Template::Template(std::string name) :
    name_(name)
{}

void Template::WriteLiberty(size_t tab, FILE* f)
{
    TAB_FPRINTF(tab, f, "index_1 (\"");

    size_t i = 0;
    for (const auto & v : index_1_) {
        fprintf(f, "%f%s", v, (i < index_1_.size() - 1) ? ", " : "");
        i++;
    }
    fprintf(f, "\")\n");

    TAB_FPRINTF(tab, f, "index_2 (\"");
    i = 0;
    for (const auto & v : index_2_) {
        fprintf(f, "%f%s", v, (i < index_2_.size() - 1) ? ", " : "");
        i++;
    }
    fprintf(f, "\")\n");
}

std::vector<double>& Template::GetIndex1()
{
    return index_1_;
}

std::vector<double>& Template::GetIndex2()
{
    return index_2_;
}

void Template::SetKind(TemplKind kind)
{
    kind_ = kind;
}

TemplKind Template::GetKind()
{
    return kind_;
}

void Template::AddIndex1(double val)
{
    index_1_.push_back(val);
}

void Template::AddIndex2(double val)
{
    index_2_.push_back(val);
}

std::string Template::GetName()
{
    return name_;
}

}
