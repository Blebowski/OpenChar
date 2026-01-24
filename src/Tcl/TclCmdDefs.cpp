
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
    "Define cell to be characterized.",

    ARG({
        // name_                    has_value_
        TclCmdOpt("-input",         true,           "Input pin or pins"),
        TclCmdOpt("-output",        true,           "Output pin or pins"),
        TclCmdOpt("cell_name",      false,          "Name of the cell")
        }),

    ARG({
        return TCL_OK;
    })
)

void RegisterTclCommands(Context *ctx)
{
    ctx->tcl_commands_.push_back({ DefineCell(ctx), DefineCellCb });

    for (const auto &p : ctx->tcl_commands_) {
        fmt::printf("%p\n", (void*)&(p.first));
        Tcl_CreateObjCommand(p.first.ctx_->interp_, p.first.name_.c_str(),
                             p.second, (void*)(&(p.first)), NULL);
    }
}

}