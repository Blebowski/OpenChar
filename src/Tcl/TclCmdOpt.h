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

#ifndef TCL_CMD_OPT_H
#define TCL_CMD_OPT_H

#include <string>

#include <tcl.h>

namespace open_char {

class TclCmdOpt {

    public:
        TclCmdOpt();
        TclCmdOpt(const bool has_value, std::string value_desc, std::string desc);

        std::string name_;
        const bool has_value_;
        const std::string value_desc_;
        const std::string desc_;

        bool isOptional() const;
        bool isSet() const;
        Tcl_Obj* objv_;
};

}

#endif