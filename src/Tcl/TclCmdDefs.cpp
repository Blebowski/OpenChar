
#include <fmt/format.h>
#include <fmt/printf.h>

#include <tcl.h>
#include <utility>

#include "TclCmdDefs.h"
#include "TclCmdOpt.h"

namespace open_char {

CREATE_TCL_COMMAND(
    DefineCell,
    "define_cell",
    {       // name_                    has_value_  is_positional_
            TclCmdOpt("-input",         true,       false)
    },
    {
        fmt::print("define_cell executed\n");
    }
)

void RegisterTclCommands(Context *ctx)
{
    std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>> tcl_commands = {
        {DefineCell(ctx), DefineCellCb}
    };

    for (const auto &p : tcl_commands) {
        Tcl_CreateObjCommand(p.first.ctx_->interp_, p.first.name_.c_str(),
                             p.second, (void*)(&p.first), NULL);
    }
}

}