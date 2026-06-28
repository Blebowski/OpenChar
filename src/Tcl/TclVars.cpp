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

#include <filesystem>

#include <tcl.h>
#include "Context.h"

#include "TclVars.h"

namespace open_char {

// TODO: Wrap debug variables only for debug build
CREATE_TCL_VAR_CB(debug_enable)
CREATE_TCL_VAR_CB(debug_expr_enable)
CREATE_TCL_VAR_CB(debug_logtbl_enable)
CREATE_TCL_VAR_CB(debug_stphld_enable)
CREATE_TCL_VAR_CB(debug_mpw_enable)

CREATE_TCL_VAR_CB(delay_in_rise)
CREATE_TCL_VAR_CB(delay_in_fall)
CREATE_TCL_VAR_CB(delay_out_rise)
CREATE_TCL_VAR_CB(delay_out_fall)
CREATE_TCL_VAR_CB(max_threads)

CREATE_TCL_VAR_CB(run_directory)

CREATE_TCL_VAR_CB(sim_timestep)

CREATE_TCL_VAR_CB(slew_lower_rise)
CREATE_TCL_VAR_CB(slew_lower_fall)
CREATE_TCL_VAR_CB(slew_upper_rise)
CREATE_TCL_VAR_CB(slew_upper_fall)


void CreateTclVariables(Context *ctx)
{
    Tcl_Interp* interp = ctx->GetTclInterp();

    // TODO: Wrap only for debug build
    REGISTER_TCL_VAR_CB(debug_enable);
    REGISTER_TCL_VAR_CB(debug_expr_enable);
    REGISTER_TCL_VAR_CB(debug_logtbl_enable);
    REGISTER_TCL_VAR_CB(debug_stphld_enable);
    REGISTER_TCL_VAR_CB(debug_mpw_enable);

    REGISTER_TCL_VAR_CB(delay_in_rise);
    REGISTER_TCL_VAR_CB(delay_in_fall);
    REGISTER_TCL_VAR_CB(delay_out_rise);
    REGISTER_TCL_VAR_CB(delay_out_fall);

    REGISTER_TCL_VAR_CB(max_threads);
    REGISTER_TCL_VAR_CB(run_directory);

    REGISTER_TCL_VAR_CB(sim_timestep);

    REGISTER_TCL_VAR_CB(slew_lower_rise);
    REGISTER_TCL_VAR_CB(slew_lower_fall);
    REGISTER_TCL_VAR_CB(slew_upper_rise);
    REGISTER_TCL_VAR_CB(slew_upper_fall);
}

}