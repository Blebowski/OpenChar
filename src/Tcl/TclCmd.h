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

#ifndef TCL_CMD_H
#define TCL_CMD_H

#include <string>
#include <map>

#include <tcl.h>

#include "Context.h"
#include "TclCmdOpt.h"

namespace open_char {

class TclCmd {

    public:
        TclCmd(Context *ctx, std::string name, std::string desc, bool print_ret_code,
               std::map<std::string, TclCmdOpt> opts);

        Context *ctx_;
        const std::string name_;
        const std::string desc_;
        const bool print_ret_code_;
        std::map<std::string, TclCmdOpt> opts_;

        int ParseArgs(Tcl_Interp* interp, int objc, Tcl_Obj* const* objv);

        template <typename Func>
        int ForEachInGroup(std::string val, Func func)
        {
            std::size_t start = 0;
            int rv = TCL_OK;

            while (true) {
                size_t pos = val.find(' ', start);

                if (pos == val.npos) {
                    rv = func(val.substr(start));
                    break;
                }

                rv = func(val.substr(start, pos - start));

                if (rv != TCL_OK)
                    return rv;

                start = pos + 1;
            }
            return rv;
        }

        void Help(void);
};

}

#endif