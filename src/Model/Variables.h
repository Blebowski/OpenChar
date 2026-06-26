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

#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <map>

#include "open_char.h"

namespace open_char {

class Variables {

    public:
        Variables();

        std::string SetVariable(std::string name, std::string value);
        std::string GetVariable(std::string name);
        double GetDoubleVariable(std::string name);
        int GetIntVariable(std::string name);
        std::map<std::string, TclVar> GetVariables();

        void PrintVariables();

    private:
        std::map<std::string, TclVar> variables_;
};

}

#endif