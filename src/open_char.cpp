
#include <iostream>

#include <fmt/format.h>
#include <fmt/printf.h>

#include <tcl.h>
#include <tclreadline.h>

int AppInit(Tcl_Interp *interp)
{
    Tcl_SetVar(interp, "tcl_prompt1",
               "puts -nonewline \"open_char> \"; flush stdout",
               TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "tcl_prompt2",
               "puts -nonewline \"... \"; flush stdout",
               TCL_GLOBAL_ONLY);

    return TCL_OK;
}

int main(int argc, char **argv)
{
    Tcl_FindExecutable(argv[0]);

    Tcl_Main(argc, argv, AppInit);
}