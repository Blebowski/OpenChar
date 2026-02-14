

#include <filesystem>
#include <cassert>

#include "Utils.h"
#include "Variables.h"


namespace open_char {

Variables::Variables() :
    variables_ ({
        {"run_directory",
            {
                .kind = VarKind::STRING,
                .s_val = std::filesystem::current_path(),
                .i_val = 0,
                .d_val = 0
            }
        },
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
        }
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
