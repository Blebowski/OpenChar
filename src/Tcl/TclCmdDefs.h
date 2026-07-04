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

#ifndef TCL_CMD_DEFS_H
#define TCL_CMD_DEFS_H

#include "TclCmd.h"
#include "Utils.h"

namespace open_char {

#define ARG(...) __VA_ARGS__

#define CREATE_TCL_COMMAND(class_name, cmd_name, desc, print_ret_code, params, exec_rtn)            \
                                                                                                    \
    class class_name : public TclCmd {                                                              \
    public:                                                                                         \
        class_name(Context *ctx);                                                                   \
        int Execute();                                                                              \
    };                                                                                              \
                                                                                                    \
    class_name::class_name(Context *ctx):                                                           \
        TclCmd (                                                                                    \
            ctx,                                                                                    \
            cmd_name,                                                                               \
            desc,                                                                                   \
            print_ret_code,                                                                         \
            params                                                                                  \
        ) {}                                                                                        \
                                                                                                    \
    int class_name::Execute()                                                                       \
    {                                                                                               \
        exec_rtn                                                                                    \
    }                                                                                               \
                                                                                                    \
    static int class_name##Cb(void *data, Tcl_Interp* interp, int objc, Tcl_Obj* const* objv)       \
    {                                                                                               \
        class_name *cmd = (class_name *) data;                                                      \
        int parse_rv = cmd->ParseArgs(interp, objc, objv);                                          \
        if (parse_rv == -1)                                                                         \
            return TCL_OK;                                                                          \
        if (parse_rv != TCL_OK)                                                                     \
            return parse_rv;                                                                        \
        int rv = cmd->Execute();                                                                    \
        if (cmd->print_ret_code_) {                                                                 \
            printf("%c\n", (rv == TCL_OK) ? '1' : '0');                                             \
            fflush(stdout);                                                                         \
        }                                                                                           \
        return rv;                                                                                  \
    }                                                                                               \

}

#endif