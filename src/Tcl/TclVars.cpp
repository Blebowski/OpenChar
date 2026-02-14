
#include <filesystem>

#include <tcl.h>
#include "Context.h"

#include "TclVars.h"

namespace open_char {

CREATE_TCL_VAR_CB(delay_in_rise)
CREATE_TCL_VAR_CB(delay_in_fall)
CREATE_TCL_VAR_CB(delay_out_rise)
CREATE_TCL_VAR_CB(delay_out_fall)

CREATE_TCL_VAR_CB(run_directory)


void CreateTclVariables(Context *ctx)
{
    Tcl_Interp* interp = ctx->GetTclInterp();

    REGISTER_TCL_VAR_CB(delay_in_rise)
    REGISTER_TCL_VAR_CB(delay_in_fall)
    REGISTER_TCL_VAR_CB(delay_out_rise)
    REGISTER_TCL_VAR_CB(delay_out_fall)

    REGISTER_TCL_VAR_CB(run_directory);
}

}