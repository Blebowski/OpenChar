
#include <fmt/format.h>
#include <fmt/printf.h>

#include <tcl.h>
#include <utility>

#include "TclCmdDefs.h"
#include "TclCmdOpt.h"

namespace open_char {

DefineCell::DefineCell(Context *ctx):
    TclCmd(
        ctx,
        "define_cell",
        {             // name_              has_value_              is_positional_
            TclCmdOpt("-input",             true,                   false)
        }
    )
{}

int DefineCell::Execute()
{
    // TODO: Execute here

    return TCL_OK;
}

int DefinceCellCb(void *data, Tcl_Interp* interp, int objc, Tcl_Obj* const* objv)
{
    DefineCell *cmd = (DefineCell *)data;

    cmd->ParseArgs(interp, objc, objv);
    cmd->Execute();

    return TCL_OK;
}

void RegisterTclCommands(Context *ctx)
{
    std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>> tcl_commands = {
        {DefineCell(ctx), DefinceCellCb}
    };

    for (const auto &p : tcl_commands) {
        Tcl_CreateObjCommand(p.first.ctx_->interp_, p.first.name_.c_str(),
                             p.second, (void*)(&p.first), NULL);
    }
}

}