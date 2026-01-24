
#include <iostream>

#include <fmt/format.h>
#include <fmt/printf.h>

#include <tcl.h>

#include "open_char.h"
#include "Context.h"

open_char::Context *ctx;

int TclInit(Tcl_Interp *interp)
{
    ctx->interp_ = interp;

    Tcl_SetVar(interp, "tcl_prompt1",
               "puts -nonewline \"open_char> \"; flush stdout",
               TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "tcl_prompt2",
               "puts -nonewline \"... \"; flush stdout",
               TCL_GLOBAL_ONLY);

    open_char::RegisterTclCommands(ctx);

    return TCL_OK;
}

int main(int argc, char **argv)
{
    ctx = new open_char::Context;

    Tcl_FindExecutable(argv[0]);
    Tcl_Main(argc, argv, TclInit);

    // TODO: This is not called since "exit" is handled within Tcl_Main to exit
    //       the code!
    delete ctx;

    return 0;
}