
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
        // name_                  has_value_      desc_
        {"-input",      TclCmdOpt(true,           "Input pin or pins")},
        {"-output",     TclCmdOpt(true,           "Output pin or pins")},
        {"cell_name",   TclCmdOpt(false,          "Name of the cell")}
        }),
    ARG({

        const std::string cell_name = Tcl_GetString((Tcl_Obj*)opts_["cell_name"].objv_);

        if (!ctx_->AddCell(cell_name)) {
            // TODO: Replace by some logging
            fmt::printf("Error: Cell %s is already defined", cell_name);
            return TCL_ERROR;
        }

        // TODO: Add Pins

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    Help,
    "help",
    "List all available commands",

    ARG({}),

    ARG({

        // TODO: Sort alphabetically
        for (const auto & cmd_pair : ctx_->tcl_commands_)
            fmt::printf("%s\n", cmd_pair.first.name_);

        return TCL_OK;
    })
)

void RegisterTclCommands(Context *ctx)
{
    ctx->tcl_commands_.push_back({ DefineCell(ctx), DefineCellCb });
    ctx->tcl_commands_.push_back({ Help(ctx),       HelpCb });

    for (const auto &p : ctx->tcl_commands_)
        Tcl_CreateObjCommand(p.first.ctx_->interp_, p.first.name_.c_str(),
                             p.second, (void*)(&(p.first)), NULL);
}

}