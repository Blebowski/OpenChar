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

#ifndef TCL_VARS_H
#define TCL_VARS_H

#include <tcl.h>

namespace open_char {

#define CREATE_TCL_VAR_CB(var_name)                                                             \
    static char *var_name##_cb(void *clientData, Tcl_Interp *interp,                            \
            const char *name1, [[maybe_unused]] const char *name2, [[maybe_unused]] int flags)  \
    {                                                                                           \
        Context* ctx = static_cast<Context*>(clientData);                                       \
        std::string val = Tcl_GetVar(interp, name1, 0);                                         \
        std::string new_val = ctx->GetVariables().SetVariable(#var_name, val);                  \
        Tcl_SetVar(interp, name1, new_val.c_str(), 0);                                          \
        return NULL;                                                                            \
    }

#define REGISTER_TCL_VAR_CB(var_name)                                                           \
    {                                                                                           \
        Tcl_TraceVar(interp, #var_name, TCL_TRACE_WRITES, var_name##_cb,                        \
                     static_cast<void*>(ctx));                                                  \
        Tcl_SetVar(interp, #var_name, ctx->GetVariables().GetVariable(#var_name).c_str(), 0);   \
    }

}

#endif