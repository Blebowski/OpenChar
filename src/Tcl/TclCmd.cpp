
#include <fmt/format.h>
#include <fmt/printf.h>

#include "TclCmd.h"

namespace open_char {

TclCmd::TclCmd(Context *ctx, std::string name, std::vector<TclCmdOpt> opts) :
    ctx_(ctx),
    name_(name),
    opts_(opts)
{}

int TclCmd::ParseArgs(Tcl_Interp* interp, int objc, Tcl_Obj* const* objv)
{
    (void) objc;
    (void) objv;
    (void) interp;

    // TODO: Parse arguments

    return TCL_OK;
}

}