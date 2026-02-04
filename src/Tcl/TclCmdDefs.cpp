
#include <tcl.h>
#include <utility>

#include "TclCmdDefs.h"
#include "TclCmdOpt.h"
#include "Supply.h"
#include "Utils.h"

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

        if (!opts_["-input"].isSet() && !opts_["-output"].isSet()) {
            error("You need to specify at least -input or -outputs.\n");
            return TCL_ERROR;
        }

        const std::string cell_name = Tcl_GetString(opts_["cell_name"].objv_);

        std::pair<Cell&, bool> cell_p = ctx_->lib_.AddCell(cell_name);
        if (!cell_p.second) {
            error("Cell %s is already defined\n", cell_name);
            return TCL_ERROR;
        }

        if (opts_["-output"].isSet()) {
            const std::string inputs = Tcl_GetString(opts_["-output"].objv_);

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

        if (opts_["-input"].isSet()) {
            const std::string inputs = Tcl_GetString(opts_["-input"].objv_);

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

        Supply *supply = ctx_->lib_.GetOpCond().supply_;
        cell_p.first.AddPin(supply->vdd_name_, PinDirection::INOUT, PinKind::PWR);
        cell_p.first.AddPin(supply->gnd_name_, PinDirection::INOUT, PinKind::GND);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    ExtractLogicTable,
    "extract_logic_table",
    "Extract logic table of a cell(s)",

    ARG({
        // name_                  has_value_      desc_
        {"cell_name",   TclCmdOpt(false,          "Name of the cell")}
        }),

    ARG({

        const std::string cell_name = Tcl_GetString(opts_["cell_name"].objv_);

        if (!ctx_->lib_.HasCell(cell_name)) {
            error("The cell %s does not exist. Use 'define_cell' to define it.\n",
                    cell_name);
            return TCL_ERROR;
        }

        ctx_->algorithms_->GetLogicFunction(ctx_->lib_.GetCell(cell_name));

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    SetVdd,
    "set_vdd",
    "Define supply voltage and supply voltage net name",

    ARG({
        // name_                      has_value_      desc_
        {"net_name",        TclCmdOpt(true,          "Name of the supply voltage net")},
        {"voltage_value",   TclCmdOpt(true,          "Supply voltage value")}
        }),

    ARG({
        double volts;
        Tcl_GetDoubleFromObj(ctx_->interp_, opts_["voltage_value"].objv_, &volts);
        std::string name = Tcl_GetString(opts_["net_name"].objv_);

        ctx_->lib_.SetDefaultSupplyVdd(name, volts);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    SetGnd,
    "set_gnd",
    "Define ground voltage and ground net name",

    ARG({
        // name_                      has_value_      desc_
        {"net_name",        TclCmdOpt(true,          "Name of the ground net")},
        {"voltage_value",   TclCmdOpt(true,          "Ground voltage value")}
        }),

    ARG({
        double volts;
        Tcl_GetDoubleFromObj(ctx_->interp_, opts_["voltage_value"].objv_, &volts);
        std::string name = Tcl_GetString(opts_["net_name"].objv_);

        ctx_->lib_.SetDefaultSupplyGnd(name, volts);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    SetOperatingCondition,
    "set_operating_condition",
    "Define operating conditions (supply volate, temperature, name)",

    ARG({
        // name_                      has_value_      desc_
        {"-name",           TclCmdOpt(true,          "Name of the operating condition.")},
        {"-supply_name",    TclCmdOpt(true,          "Supply name set by 'set_vdd' command.")},
        {"-temp",           TclCmdOpt(true,          "Operating temperature.")},
        {"-voltage",        TclCmdOpt(true,          "Operating voltage.")}
        }),

    ARG({

        if (!opts_["-name"].isSet()) {
            error("You need to specify -name for operating conditions name\n");
            return TCL_ERROR;
        }

        if (!opts_["-temp"].isSet()) {
            error("You need to specify -temp for operating conditions temperature\n");
            return TCL_ERROR;
        }

        if ((!opts_["-supply_name"].isSet() && !opts_["-voltage"].isSet()) ||
            (opts_["-supply_name"].isSet() && opts_["-voltage"].isSet())) {
            error("You need to specify exactly one of: -supply_name, -voltage .\n");
            return TCL_ERROR;
        }

        if (opts_["-supply_name"].isSet()) {
            std::string supply_name = Tcl_GetString(opts_["-supply_name"].objv_);
            if (!ctx_->lib_.HasSupply(supply_name)) {
                error("Supply %s does not exists. Define it using set_vdd", supply_name);
                return TCL_ERROR;
            }
        }

        ctx_->lib_.GetOpCond().name_ = Tcl_GetString(opts_["-name"].objv_);

        double temp;
        Tcl_GetDoubleFromObj(ctx_->interp_, opts_["-temp"].objv_, &temp);
        ctx_->lib_.GetOpCond().temp_ = temp;

        if (opts_["-voltage"].isSet()) {
            double volts;
            Tcl_GetDoubleFromObj(ctx_->interp_, opts_["voltage"].objv_, &volts);
            ctx_->lib_.SetDefaultSupplyVdd(volts);
        }

        if (opts_["-supply_name"].isSet()) {
            std::string supply_name = Tcl_GetString(opts_["-supply_name"].objv_);
            Supply& supply = ctx_->lib_.GetSupply(supply_name);
            ctx_->lib_.GetOpCond().supply_ = &(supply);
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
            printf("%s\n", cmd_pair.first.name_);

        return TCL_OK;
    })
)

void RegisterTclCommands(Context *ctx)
{
    ctx->tcl_commands_.push_back({ DefineCell(ctx),             DefineCellCb });
    ctx->tcl_commands_.push_back({ ExtractLogicTable(ctx),      ExtractLogicTableCb });
    ctx->tcl_commands_.push_back({ SetVdd(ctx),                 SetVddCb });
    ctx->tcl_commands_.push_back({ SetGnd(ctx),                 SetGndCb });
    ctx->tcl_commands_.push_back({ SetOperatingCondition(ctx),  SetOperatingConditionCb });
    ctx->tcl_commands_.push_back({ Help(ctx),                   HelpCb });

    for (const auto &p : ctx->tcl_commands_)
        Tcl_CreateObjCommand(p.first.ctx_->interp_, p.first.name_.c_str(),
                             p.second, (void*)(&(p.first)), NULL);
}

}