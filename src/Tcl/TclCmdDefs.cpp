
#include <tcl.h>
#include <utility>

#include "TclCmdDefs.h"
#include "TclCmdOpt.h"
#include "Supply.h"
#include "Template.h"

#include "float.h"

namespace open_char {

CREATE_TCL_COMMAND(
    DefineCell,
    "define_cell",
    "Define cell to be characterized.",
    true,

    ARG({
        {"-input",      TclCmdOpt(true,         "{pin_names}",      "Input pin or pins")},
        {"-output",     TclCmdOpt(true,         "{pin_names}",      "Output pin or pins")},
        {"cell_name",   TclCmdOpt(false,        "cell_name",        "Name of the cell")}
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
            const std::string s = Tcl_GetString(opts_["-output"].objv_);

            // TODO: Handle duplicit pins here!
            ForEachInGroup(s, [&](const std::string &val){
                cell_p.first.AddPin(val, PinDirection::OUT, PinKind::DATA);
                return TCL_OK;
            });
        }

        if (opts_["-input"].isSet()) {
            const std::string s = Tcl_GetString(opts_["-input"].objv_);

            // TODO: Handle duplicit pins here!
            ForEachInGroup(s, [&](const std::string &val){
                cell_p.first.AddPin(val, PinDirection::IN, PinKind::DATA);
                return TCL_OK;
            });
        }

        Supply *supply = ctx_->lib_.GetOpCond().supply_;
        cell_p.first.AddPin(supply->vdd_name_, PinDirection::INOUT, PinKind::PWR);
        cell_p.first.AddPin(supply->gnd_name_, PinDirection::INOUT, PinKind::GND);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    DefineTemplate,
    "define_template",
    "Defines a template for characterization.",
    true,

    ARG({
        {"-type",         TclCmdOpt(true,       "{delay|power}",    "Type of template to be defined.")},
        {"-index_1",      TclCmdOpt(true,       "{values}",         "List of values to be used as first index.")},
        {"-index_2",      TclCmdOpt(true,       "{values}",         "List of values to be used as second index.")},
        {"template_name", TclCmdOpt(true,       "",                 "Name of the template.")},
        }),

    ARG({

        if (!opts_["-type"].isSet()) {
            error("You need to specify -type for the template.\n");
            return TCL_ERROR;
        }

        if (!opts_["-index_1"].isSet()) {
            error("You need to specify -index_1 for the template.\n");
            return TCL_ERROR;
        }

        if (!opts_["template_name"].isSet()) {
            error("You need to specify template_name for the template.\n");
            return TCL_ERROR;
        }

        std::string type = Tcl_GetString(opts_["-type"].objv_);
        if (type != "delay" && type != "power") {
            error("Allowed values for -type are: 'delay' or 'power'.\n");
            return TCL_ERROR;
        }

        std::pair<Template&, bool> template_p =
            ctx_->lib_.AddTemplate(Tcl_GetString(opts_["template_name"].objv_));

        if (!template_p.second) {
            error("Template %s already exists.\n", template_p.first.name_);
            return TCL_ERROR;
        }

        if (type == "delay") {
            template_p.first.kind_ = TemplateKind::DELAY;
        } else {
            template_p.first.kind_ = TemplateKind::POWER;
        }

        if (opts_["-index_1"].isSet()) {
            const std::string s = Tcl_GetString(opts_["-index_1"].objv_);
            double min = -DBL_MAX;

            int rv = ForEachInGroup(s, [&](const std::string &val){
                char *end;
                double v = strtof(val.c_str(), &end);
                if (*end != '\0') {
                    error("%s is not float value in definition of -index_1\n", val);
                    return TCL_ERROR;
                }
                if (v <= min) {
                    error("Value %f is not larger than previous value %f. "
                          "Index values shall be monotonically increasing.\n",
                           v, min);
                    return TCL_ERROR;
                }
                min = v;
                template_p.first.i1.push_back(atof(val.c_str()));
                return TCL_OK;
            });

            if (rv != TCL_OK) {
                error("-index_1 value is invalid.\n");
                return TCL_ERROR;
            }
        }

        if (opts_["-index_2"].isSet()) {
            const std::string s = Tcl_GetString(opts_["-index_2"].objv_);
            double min = -DBL_MAX;

            int rv = ForEachInGroup(s, [&](const std::string &val){
                char *end;
                double v = strtof(val.c_str(), &end);
                if (*end != '\0') {
                    error("%s is not float value in definition of -index_2\n", val);
                    return TCL_ERROR;
                }
                if (v <= min) {
                    error("Value %f is not larger than previous value %f. "
                          "Index values shall be monotonically increasing.\n",
                          v, min);
                    return TCL_ERROR;
                }
                min = v;
                template_p.first.i2.push_back(atof(val.c_str()));
                return TCL_OK;
            });

            if (rv != TCL_OK) {
                error("-index_2 value is invalid.\n");
                return TCL_ERROR;
            }
        }

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    ExtractLogicTable,
    "extract_logic_table",
    "Extract logic table of a cell(s)",
    true,

    ARG({
        {"cell_name",   TclCmdOpt(true,     "",           "Name of the cell")}
        }),

    ARG({

        const std::string cell_name = Tcl_GetString(opts_["cell_name"].objv_);

        if (!ctx_->lib_.HasCell(cell_name)) {
            error("The cell %s does not exist. Use 'define_cell' to define it.\n",
                    cell_name);
            return TCL_ERROR;
        }

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    ReadSpice,
    "read_spice",
    "Read SPICE netlist(s) or model deck(s)",
    true,

    ARG({
        {"-type",           TclCmdOpt(true,     "(netlist|model)",  "Type of the file provided.")},
        {"{spice_files}",   TclCmdOpt(true,     "",                 "Name of the supply voltage net.")}
        }),

    ARG({

        std::string type = Tcl_GetString(opts_["-type"].objv_);
        if (type != "netlist" && type != "model") {
            error("Allowed values for -type are: 'netlist' or 'model'.\n");
            return TCL_ERROR;
        }

        const std::string s = Tcl_GetString(opts_["-type"].objv_);

        ForEachInGroup(s, [&](const std::string &val){
            // TODO: Check existence of the file
            if (type == "netlist")
                ctx_->includes_.push_back(val);
            else
                ctx_->models_.push_back(val);
            return TCL_OK;
        });

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    SetVdd,
    "set_vdd",
    "Define supply voltage and supply voltage net name",
    true,

    ARG({
        {"net_name",        TclCmdOpt(true,     "",   "Name of the supply voltage net")},
        {"voltage_value",   TclCmdOpt(true,     "",   "Supply voltage value")}
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
    true,

    ARG({
        {"net_name",        TclCmdOpt(true,     "",     "Name of the ground net")},
        {"voltage_value",   TclCmdOpt(true,     "",     "Ground voltage value")}
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
    true,

    ARG({
        {"-name",           TclCmdOpt(true,     "string",       "Name of the operating condition.")},
        {"-supply_name",    TclCmdOpt(true,     "string",       "Supply name set by 'set_vdd' command.")},
        {"-temp",           TclCmdOpt(true,     "float",        "Operating temperature.")},
        {"-voltage",        TclCmdOpt(true,     "float",        "Operating voltage.")}
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
    false,

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
    ctx->tcl_commands_.push_back({ DefineTemplate(ctx),         DefineTemplateCb });
    ctx->tcl_commands_.push_back({ ExtractLogicTable(ctx),      ExtractLogicTableCb });
    ctx->tcl_commands_.push_back({ ReadSpice(ctx),              ReadSpiceCb });
    ctx->tcl_commands_.push_back({ SetVdd(ctx),                 SetVddCb });
    ctx->tcl_commands_.push_back({ SetGnd(ctx),                 SetGndCb });
    ctx->tcl_commands_.push_back({ SetOperatingCondition(ctx),  SetOperatingConditionCb });
    ctx->tcl_commands_.push_back({ Help(ctx),                   HelpCb });

    for (const auto &p : ctx->tcl_commands_) {
        std::string full_name = "open_char::" + p.first.name_;
        Tcl_CreateObjCommand(p.first.ctx_->interp_, full_name.c_str(),
                             p.second, (void*)(&(p.first)), NULL);
    }
}

}