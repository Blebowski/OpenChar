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

#include <filesystem>
#include <cassert>
#include <thread>

#include "Utils.h"
#include "Variables.h"

namespace open_char {

static bool is_within_0_1(std::string name, std::string val)
{
    double d_val = std::stof(val);
    if (d_val < 0 || d_val > 1) {
        error("invalid value '%f' for variable %s. "
              "The value shall be between 0 and 1 inclusive.",
              val, name);
        return false;
    }

    return true;
}

static bool is_positive_float(std::string name, std::string val)
{
    double d_val = std::stof(val);
    if (d_val < 0) {
        error("invalid value '%f' for variable %s. "
              "The value shall be positive float number",
              val, name);
        return false;
    }

    return true;
}

static bool is_positive_int(std::string name, std::string val)
{
    int i_val = std::stoi(val);
    if (i_val < 0) {
        error("invalid value '%d' for variable %s. "
              "The value shall be positive integer",
              val, name);
        return false;
    }

    return true;
}

#define SET_GLOBAL_VAR(var_name)                                \
    [] ([[maybe_unused]] std::string name, std::string val) {   \
        var_name = static_cast<bool>(std::stoi(val));           \
        return true;                                            \
    }

Variables::Variables() :
    variables_ ({

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Application variables - to be exposed to the user
        ///////////////////////////////////////////////////////////////////////////////////////////
        {"delay_in_rise",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5,
                .validate = &is_within_0_1
            }
        },
        {"delay_in_fall",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5,
                .validate = &is_within_0_1
            }
        },
        {"delay_out_rise",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5,
                .validate = &is_within_0_1
            }
        },
        {"delay_out_fall",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5,
                .validate = &is_within_0_1
            }
        },
        {"max_threads",
            {
                .kind = TclVarKind::INT,
                .s_val = "",
                .i_val = static_cast<int>(std::thread::hardware_concurrency()),
                .d_val = 0,
                .validate = &is_positive_int
            }
        },
        {"run_directory",
            {
                .kind = TclVarKind::STRING,
                .s_val = std::filesystem::current_path(),
                .i_val = 0,
                .d_val = 0,
                .validate = nullptr
            }
        },
        {"sim_timestep",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.001,
                .validate = &is_positive_float
            }
        },
        {"slew_lower_fall",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.2,
                .validate = &is_within_0_1
            }
        },
        {"slew_upper_fall",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.8,
                .validate = &is_within_0_1
            }
        },
        {"slew_lower_rise",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.2,
                .validate = &is_within_0_1
            }
        },
        {"slew_upper_rise",
            {
                .kind = TclVarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.8,
                .validate = &is_within_0_1
            }
        },

        ///////////////////////////////////////////////////////////////////////////////////////////
        // Debug variables - only for internal development usage
        ///////////////////////////////////////////////////////////////////////////////////////////
        // TODO: Wrap the debug variables to be present only in debug build!
        {"debug_enable",
            {
                .kind = TclVarKind::INT,
                .s_val = "",
                .i_val = 0,
                .d_val = 0,
                .validate = [] ([[maybe_unused]] std::string name, std::string val) {
                    debug_enable = static_cast<bool>(std::stoi(val));
                    return true;
                },
            }
        },
        {"debug_expr_enable",
            {
                .kind = TclVarKind::INT,
                .s_val = "",
                .i_val = 0,
                .d_val = 0,
                .validate = SET_GLOBAL_VAR(debug_expr_enable)
            }
        },
        {"debug_logtbl_enable",
            {
                .kind = TclVarKind::INT,
                .s_val = "",
                .i_val = 0,
                .d_val = 0,
                .validate = SET_GLOBAL_VAR(debug_logtbl_enable)
            }
        },
        {"debug_stphld_enable",
            {
                .kind = TclVarKind::INT,
                .s_val = "",
                .i_val = 0,
                .d_val = 0,
                .validate = SET_GLOBAL_VAR(debug_stphld_enable)
            }
        },
        {"debug_mpw_enable",
            {
                .kind = TclVarKind::INT,
                .s_val = "",
                .i_val = 0,
                .d_val = 0,
                .validate = SET_GLOBAL_VAR(debug_mpw_enable)
            }
        }
    })
{}

std::string Variables::SetVariable(std::string name, std::string value)
{
    assert (variables_.contains(name));
    TclVar& v = variables_[name];

    double d_val = 0.0;
    int i_val = 0;
    try {
        if (v.kind == TclVarKind::DOUBLE) {
            d_val = std::stof(value);
        } else if (v.kind == TclVarKind::INT) {
            i_val = std::stoi(value);
        }
    } catch (...) {
        error("Invalid value '%s' for variable %s whose type is %s.",
            value, name, toString(v.kind));
        return v.s_val;
    }

    if (v.validate) {
        bool is_ok = v.validate(name, value);
        if (!is_ok) {
            return v.s_val;
        }
    }

    if (v.kind == TclVarKind::DOUBLE) {
        v.s_val = value;
        v.d_val = d_val;
    } else if (v.kind == TclVarKind::INT) {
        v.s_val = value;
        v.i_val = i_val;
    } else {
        v.s_val = value;
    }

    return value;
}

std::string Variables::GetVariable(std::string name)
{
    TclVar& v = variables_[name];

    if (v.kind == TclVarKind::DOUBLE) {
        return std::to_string(v.d_val);
    } else if (v.kind == TclVarKind::INT) {
        return std::to_string(v.i_val);
    }

    return v.s_val;
}

double Variables::GetDoubleVariable(std::string name)
{
    TclVar& v = variables_[name];
    assert (v.kind == TclVarKind::DOUBLE);

    return v.d_val;
}

int Variables::GetIntVariable(std::string name)
{
    TclVar& v = variables_[name];
    assert (v.kind == TclVarKind::INT);

    return v.i_val;
}

std::map<std::string, TclVar> Variables::GetVariables()
{
    return variables_;
}

void Variables::PrintVariables()
{
    for (const auto &r : variables_) {
        printf("%-25s", r.first);
        if (r.second.kind == TclVarKind::DOUBLE) {
            printf("%f\n", r.second.d_val);
        } else if (r.second.kind == TclVarKind::INT) {
            printf("%d\n", r.second.i_val);
        } else {
            printf("%s\n", r.second.s_val);
        }
    }
}

}
