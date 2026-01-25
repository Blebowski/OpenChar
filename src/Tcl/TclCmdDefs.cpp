
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

        if (opts_["-input"].objv_ == nullptr && opts_["-output"].objv_ == nullptr) {
            // TODO: Replace by some logging
            fmt::printf("Error: You need to specify at least -input or -outputs.\n");
            return TCL_ERROR;
        }

        const std::string cell_name = Tcl_GetString((Tcl_Obj*)opts_["cell_name"].objv_);

        std::pair<Cell&, bool> cell_p = ctx_->AddCell(cell_name);
        if (!cell_p.second) {
            // TODO: Replace by some logging
            fmt::printf("Error: Cell %s is already defined\n", cell_name);
            return TCL_ERROR;
        }

        if (opts_["-input"].objv_ != nullptr) {
            const std::string inputs = Tcl_GetString((Tcl_Obj*)opts_["-input"].objv_);

            // TODO: Move this to some function that can process either single element or collection!
            // TODO: Handle duplicit pins here!
            std::size_t start = 0;
            while (true) {
                size_t pos = inputs.find(' ', start);
                if (pos == inputs.npos) {
                    cell_p.first.AddPin(inputs.substr(start), PinDirection::IN, PinKind::DATA);
                    break;
                }
                cell_p.first.AddPin(inputs.substr(start, pos - start),
                                    PinDirection::IN, PinKind::DATA);
                start = pos + 1;
            }
        }

        if (opts_["-output"].objv_ != nullptr) {
            const std::string inputs = Tcl_GetString((Tcl_Obj*)opts_["-output"].objv_);

            // TODO: Move this to some function that can process either single element or collection!
            // TODO: Handle duplicit pins here!
            std::size_t start = 0;
            while (true) {
                size_t pos = inputs.find(' ', start);
                if (pos == inputs.npos) {
                    cell_p.first.AddPin(inputs.substr(start), PinDirection::OUT, PinKind::DATA);
                    break;
                }
                cell_p.first.AddPin(inputs.substr(start, pos - start),
                                    PinDirection::OUT, PinKind::DATA);
                start = pos + 1;
            }
        }

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