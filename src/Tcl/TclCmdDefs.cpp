////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tcl.h>
#include <utility>

#include "TclCmdDefs.h"
#include "TclCmdOpt.h"
#include "Supply.h"
#include "Template.h"

#include "float.h"

namespace open_char {

CREATE_TCL_COMMAND(
    CharacterizeLibrary,
    "characterize_library",
    "Characterize defined cells into a library.",
    true,

    ARG({}),
    ARG({
        if (ctx_->GetAlgorithms().CharacterizeLibrary()) {
            return TCL_OK;
        }
        return TCL_ERROR;
    })
)

CREATE_TCL_COMMAND(
    DefineCell,
    "define_cell",
    "Define a cell to be characterized.",
    true,

    ARG({
        {"-input",      TclCmdOpt(true,         "{pin_names}",          "Input pin or pins.")},
        {"-output",     TclCmdOpt(true,         "{pin_names}",          "Output pin or pins.")},
        {"-clock",      TclCmdOpt(true,         "pin_name",             "Clock pin")},
        {"-async",      TclCmdOpt(true,         "{pin_names}",          "Asynchronous set/clear pin or pins.")},
        {"-delay",      TclCmdOpt(true,         "delay_template",       "Name of delay template to use.")},
        {"-constraint", TclCmdOpt(true,         "constraint_template",  "Name of constraint template to use.")},
        {"-area",       TclCmdOpt(true,         "number",               "Cell area (in square microns).")},
        {"-footprint",  TclCmdOpt(true,         "footprint_name",       "Cell footprint.")},
        {"cell_name",   TclCmdOpt(false,        "cell_name",            "Name of the cell.")}
        }),
    ARG({

        if (!opts_["-delay"].isSet()) {
            error("You need to specify -delay template.\n");
            return TCL_ERROR;
        }

        if (opts_["-clock"].isSet() && !opts_["-constraint"].isSet()) {
            error("Cells that contain -clock pins must have -constraint template set.\n");
            return TCL_ERROR;
        }

        const std::string d_templ_name = Tcl_GetString(opts_["-delay"].objv_);
        if (!ctx_->GetLibrary().HasTemplate(d_templ_name)) {
            error("Template '%s' does not exist.", d_templ_name);
            return TCL_ERROR;
        }

        Template& t = ctx_->GetLibrary().GetTemplate(d_templ_name);
        if (t.GetKind() != TemplKind::DELAY) {
            error("Template '%s' is not a delay template.", d_templ_name);
            return TCL_ERROR;
        }

        if (opts_["-async"].isSet() && !opts_["-clock"].isSet()) {
            error("Can't specify '-async' pins without clock pin.");
            return TCL_ERROR;
        }

        if (opts_["-constraint"].isSet() && !opts_["-clock"].isSet()) {
            error("Can't specify '-constraint' pins without clock pin.");
            return TCL_ERROR;
        }

        std::string c_templ_name;
        if (opts_["-constraint"].isSet()) {
            c_templ_name = Tcl_GetString(opts_["-constraint"].objv_);

            if (!ctx_->GetLibrary().HasTemplate(c_templ_name)) {
                error("Template '%s' does not exist.", c_templ_name);
                return TCL_ERROR;
            }

            Template& t = ctx_->GetLibrary().GetTemplate(c_templ_name);

            if (t.GetKind() != TemplKind::CONSTRAINT) {
                error("Template '%s' is not a constraint template.", c_templ_name);
                return TCL_ERROR;
            }
        }

        double area = 0.0;
        if (opts_["-area"].isSet()) {
            char *end;
            std::string val = std::string(Tcl_GetString(opts_["-area"].objv_));
            area = strtof(val.c_str(), &end);
            if (*end != '\0') {
                error("%s is not float value in definition of -area\n", val);
                return TCL_ERROR;
            }
            if (area < 0.0) {
                error("-area can't be negative (%f)\n", area);
                return TCL_ERROR;
            }
        }

        std::string footprint = "";
        if (opts_["-footprint"].isSet()) {
            footprint = std::string(Tcl_GetString(opts_["-footprint"].objv_));
        }

        const std::string cell_name = Tcl_GetString(opts_["cell_name"].objv_);

        auto [cell, was_added] = ctx_->GetLibrary().AddCell(cell_name);
        if (!was_added) {
            error("Cell %s is already defined\n", cell_name);
            return TCL_ERROR;
        }

        cell.SetArea(area);
        cell.SetFootprint(footprint);
        cell.SetSupply(ctx_->GetLibrary().GetOpCond().GetSupply());
        cell.SetDelayTemplate(&(ctx_->GetLibrary().GetTemplate(d_templ_name)));

        if (opts_["-constraint"].isSet()) {
            cell.SetConstraintTemplate(&(ctx_->GetLibrary().GetTemplate(c_templ_name)));
        }

        if (opts_["-output"].isSet()) {
            const std::string s = Tcl_GetString(opts_["-output"].objv_);
            // TODO: Handle duplicit pins here!
            ForEachInGroup(s, [&](const std::string &val){
                cell.AddPin(val, PinDir::OUT, PinKind::DATA);
                return TCL_OK;
            });
        }

        if (opts_["-input"].isSet()) {
            const std::string s = Tcl_GetString(opts_["-input"].objv_);
            // TODO: Handle duplicit pins here!
            ForEachInGroup(s, [&](const std::string &val){
                cell.AddPin(val, PinDir::IN, PinKind::DATA);
                return TCL_OK;
            });
        }

        if (opts_["-async"].isSet()) {
            const std::string s = Tcl_GetString(opts_["-async"].objv_);
            // TODO: Handle duplicit pins here!
            ForEachInGroup(s, [&](const std::string &val){
                cell.AddPin(val, PinDir::IN, PinKind::ASYNC);
                return TCL_OK;
            });
        }

        if (opts_["-clock"].isSet()) {
            const std::string s = Tcl_GetString(opts_["-clock"].objv_);
            cell.AddPin(s, PinDir::IN, PinKind::CLK);
            cell.SetKind(CellKind::SEQUENTIAL);

            auto & c_pin = cell.GetPins(PinKind::CLK).front();
            cell.GetSequential().SetClockPin(&c_pin);

            // TODO: Check here the flop cell is consistent:
            //      - Has input data pin
            //      - Has output data pin

        } else {
            cell.SetKind(CellKind::COMBINATIONAL);
        }

        Supply *supply = ctx_->GetLibrary().GetOpCond().GetSupply();
        cell.AddPin(supply->GetVddName(), PinDir::INOUT, PinKind::PWR);
        cell.AddPin(supply->GetGndName(), PinDir::INOUT, PinKind::GND);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    DefineTemplate,
    "define_template",
    "Defines a template for characterization.",
    true,

    ARG({
        {"-type",         TclCmdOpt(true,       "(delay|power)",    "Type of template to be defined.")},
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

        if (!opts_["-index_2"].isSet()) {
            error("You need to specify -index_2 for the template.\n");
            return TCL_ERROR;
        }

        if (!opts_["template_name"].isSet()) {
            error("You need to specify template_name for the template.\n");
            return TCL_ERROR;
        }

        std::string type = Tcl_GetString(opts_["-type"].objv_);
        if (type != "delay" && type != "power" && type != "constraint") {
            error("Allowed values for -type are: 'delay', 'power' or 'constraint'.\n");
            return TCL_ERROR;
        }

        std::pair<Template&, bool> template_p =
            ctx_->GetLibrary().AddTemplate(Tcl_GetString(opts_["template_name"].objv_));

        if (!template_p.second) {
            error("Template %s already exists.\n", template_p.first.GetName());
            return TCL_ERROR;
        }

        if (type == "delay") {
            template_p.first.SetKind(TemplKind::DELAY);
        } else if (type == "power") {
            template_p.first.SetKind(TemplKind::POWER);
        } else {
            template_p.first.SetKind(TemplKind::CONSTRAINT);
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
                template_p.first.AddIndex1(atof(val.c_str()));
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
                template_p.first.AddIndex2(atof(val.c_str()));
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
    ReadSpice,
    "read_spice",
    "Read SPICE netlist(s) or model deck(s).",
    true,

    ARG({
        {"-type",           TclCmdOpt(true,     "(netlist|model)",  "Type of the file provided.")},
        {"-corner",         TclCmdOpt(true,     "corner",           "Corner in the model to be used (e.g. tt,ss,ff).")},
        {"{spice_files}",   TclCmdOpt(true,     "",                 "Name of the supply voltage net.")}
        }),

    ARG({

        std::string type = Tcl_GetString(opts_["-type"].objv_);
        if (type != "netlist" && type != "model") {
            error("Allowed values for -type are: 'netlist' or 'model'.\n");
            return TCL_ERROR;
        }

        // TODO: Check if corner is only specified with "model" type!

        const std::string s = Tcl_GetString(opts_["{spice_files}"].objv_);

        ForEachInGroup(s, [&](const std::string &val){
            // TODO: Check existence of the file
            if (type == "netlist") {
                ctx_->AddNetlist(val);
            } else {
                std::string full_val = val;
                if (opts_["-corner"].isSet()) {
                    full_val += " ";
                    full_val += Tcl_GetString(opts_["-corner"].objv_);
                }
                ctx_->AddModel(full_val);
            }
            return TCL_OK;
        });

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    ReportAppVars,
    "report_app_vars",
    "Report application variables and their values.",
    true,

    ARG({}),

    ARG({
        ctx_->GetVariables().PrintVariables();
        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    SetVdd,
    "set_vdd",
    "Define supply voltage and supply voltage net name.",
    true,

    ARG({
        {"net_name",        TclCmdOpt(true,     "",   "Name of the supply voltage net")},
        {"voltage_value",   TclCmdOpt(true,     "",   "Supply voltage value")}
        }),

    ARG({
        Volt volts;
        Tcl_GetDoubleFromObj(ctx_->GetTclInterp(), opts_["voltage_value"].objv_, &volts);
        std::string name = Tcl_GetString(opts_["net_name"].objv_);

        ctx_->GetLibrary().SetDefaultSupplyVdd(name, volts);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    SetGnd,
    "set_gnd",
    "Define ground voltage and ground net name.",
    true,

    ARG({
        {"net_name",        TclCmdOpt(true,     "",     "Name of the ground net")},
        {"voltage_value",   TclCmdOpt(true,     "",     "Ground voltage value")}
        }),

    ARG({
        Volt volts;
        Tcl_GetDoubleFromObj(ctx_->GetTclInterp(), opts_["voltage_value"].objv_, &volts);
        std::string name = Tcl_GetString(opts_["net_name"].objv_);

        ctx_->GetLibrary().SetDefaultSupplyGnd(name, volts);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    SetOperatingCondition,
    "set_operating_condition",
    "Define operating conditions (supply voltage, temperature, name).",
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
            if (!ctx_->GetLibrary().HasSupply(supply_name)) {
                error("Supply %s does not exists. Define it using set_vdd", supply_name);
                return TCL_ERROR;
            }
        }

        OpCond &op = ctx_->GetLibrary().GetOpCond();
        op.SetName(Tcl_GetString(opts_["-name"].objv_));

        Celsius temp;
        Tcl_GetDoubleFromObj(ctx_->GetTclInterp(), opts_["-temp"].objv_, &temp);
        op.SetTemperature(temp);

        if (opts_["-voltage"].isSet()) {
            Volt volts;
            Tcl_GetDoubleFromObj(ctx_->GetTclInterp(), opts_["voltage"].objv_, &volts);
            ctx_->GetLibrary().SetDefaultSupplyVdd(volts);
        }

        if (opts_["-supply_name"].isSet()) {
            std::string supply_name = Tcl_GetString(opts_["-supply_name"].objv_);
            Supply& supply = ctx_->GetLibrary().GetSupply(supply_name);
            op.SetSupply(&(supply));
        }

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    Help,
    "help",
    "Show all available commands.",
    false,

    ARG({}),

    ARG({
        // TODO: Sort alphabetically
        for (const auto & c : ctx_->GetTclCommands())
            printf("%s\n", c.first.name_);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    WriteLibrary,
    "write_library",
    "Write characterized library to a liberty file.",
    true,

    ARG({
        {"liberty_file",            TclCmdOpt(true,     "string",       "Output library file name.")},
        }),
    ARG({

        std::string name = Tcl_GetString(opts_["library_name"].objv_);
        ctx_->GetLibrary().WriteLiberty(name);

        return TCL_OK;
    })
)

CREATE_TCL_COMMAND(
    WriteVerilog,
    "write_verilog",
    "Create verilog model file for the cells in the library",
    true,

    ARG({
        {"output_file",            TclCmdOpt(true,     "string",       "Output verilog file name.")},
        }),
    ARG({

        std::string name = Tcl_GetString(opts_["output_file"].objv_);
        ctx_->GetLibrary().WriteVerilog(name);

        return TCL_OK;
    })
)

void CreateTclCommands(Context *ctx)
{
    ctx->AddTclCommand(CharacterizeLibrary(ctx),    CharacterizeLibraryCb );
    ctx->AddTclCommand(DefineCell(ctx),             DefineCellCb );
    ctx->AddTclCommand(DefineTemplate(ctx),         DefineTemplateCb );
    ctx->AddTclCommand(ReadSpice(ctx),              ReadSpiceCb );
    ctx->AddTclCommand(ReportAppVars(ctx),          ReportAppVarsCb );
    ctx->AddTclCommand(SetVdd(ctx),                 SetVddCb );
    ctx->AddTclCommand(SetGnd(ctx),                 SetGndCb );
    ctx->AddTclCommand(SetOperatingCondition(ctx),  SetOperatingConditionCb );
    ctx->AddTclCommand(WriteLibrary(ctx),           WriteLibraryCb );
    ctx->AddTclCommand(WriteVerilog(ctx),           WriteVerilogCb );
    ctx->AddTclCommand(Help(ctx),                   HelpCb );
}

void RegisterTclCommands(Context *ctx)
{
    for (const auto &c : ctx->GetTclCommands()) {
        std::string full_name = "open_char::" + c.first.name_;
        Tcl_CreateObjCommand(c.first.ctx_->GetTclInterp(), full_name.c_str(),
                             c.second, (void*)(&(c.first)), NULL);
    }
}

}