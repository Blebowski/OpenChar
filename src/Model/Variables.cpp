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

Variables::Variables() :
    variables_ ({
        {"delay_in_rise",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5
            }
        },
        {"delay_in_fall",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5
            }
        },
        {"delay_out_rise",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5
            }
        },
        {"delay_out_fall",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.5
            }
        },
        {"max_threads",
            {
                .kind = VarKind::INT,
                .s_val = "",
                .i_val = static_cast<int>(std::thread::hardware_concurrency()),
                .d_val = 0
            }
        },
        {"run_directory",
            {
                .kind = VarKind::STRING,
                .s_val = std::filesystem::current_path(),
                .i_val = 0,
                .d_val = 0
            }
        },
        {"slew_lower_fall",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.2
            }
        },
        {"sim_timestep",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.001
            }
        },
        {"slew_upper_fall",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.8
            }
        },
        {"slew_lower_rise",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.2
            }
        },
        {"slew_upper_rise",
            {
                .kind = VarKind::DOUBLE,
                .s_val = "",
                .i_val = 0,
                .d_val = 0.8
            }
        },
    })
{}


std::string Variables::SetVariable(std::string name, std::string value)
{
    Var& v = variables_[name];

    // TODO: Put here per variable hook that validates the data to be set (e.g. range)

    if (v.kind == VarKind::DOUBLE) {
        std::string rv = value;
        try {
            v.d_val = stof(value);
        } catch (...) {
            error("Invalid value '%s' for variable %s. Value shall be float. Setting %s to 0.",
                   value, name, name);
            rv = 0.0;
            v.d_val = 0;
        }
        return rv;
    } else if (v.kind == VarKind::INT) {
        std::string rv = value;
        try {
            v.i_val = stoi(value);
        } catch (...) {
            error("Invalid value '%s' for variable %s. Value shall be integer. Setting %s to 0.",
                  value, name, name);
            rv = 0.0;
            v.i_val = 0;
        }
        return rv;
    }

    v.s_val = value;
    return value;
}

std::string Variables::GetVariable(std::string name)
{
    Var& v = variables_[name];

    if (v.kind == VarKind::DOUBLE) {
        return std::to_string(v.d_val);
    } else if (v.kind == VarKind::INT) {
        return std::to_string(v.i_val);
    }

    return v.s_val;
}

double Variables::GetDoubleVariable(std::string name)
{
    Var& v = variables_[name];
    assert (v.kind == VarKind::DOUBLE);

    return v.d_val;
}

int Variables::GetIntVariable(std::string name)
{
    Var& v = variables_[name];
    assert (v.kind == VarKind::INT);

    return v.i_val;
}

void Variables::PrintVariables()
{
    for (const auto &r : variables_) {
        printf("%-25s", r.first);
        if (r.second.kind == VarKind::DOUBLE) {
            printf("%f\n", r.second.d_val);
        } else if (r.second.kind == VarKind::INT) {
            printf("%d\n", r.second.i_val);
        } else {
            printf("%s\n", r.second.s_val);
        }
    }
}

}
