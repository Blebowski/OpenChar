
#include <filesystem>

#include <tcl.h>
#include "Context.h"

#include "TclVars.h"

namespace open_char {

CREATE_TCL_VAR_CB("run_directory", RunDirectory)


void CreateTclVariables(Context *ctx)
{
    Tcl_Interp* interp = ctx->GetTclInterp();

    REGISTER_TCL_VAR_CB("run_directory",    RunDirectory);
}

}