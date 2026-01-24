
#include <fmt/format.h>
#include <fmt/printf.h>

#include "TclCmd.h"

namespace open_char {

TclCmd::TclCmd(Context *ctx, const char *name, const char *desc,
                std::vector<TclCmdOpt> opts) :
    ctx_(ctx),
    name_(name),
    desc_(desc),
    opts_(opts)
{}

void TclCmd::Help(void)
{
    fmt::printf("%s\n\n", desc_);

    bool has_opts = false;
    for (const auto & opt : opts_) {
        if (opt.isOptional()) {
            has_opts = true;
            break;
        }
    }

    fmt::printf("%s %s", name_, (has_opts) ? "[options]" : "");
    for (const auto & opt : opts_) {
        if (opt.isOptional())
            continue;
        fmt::printf(" %s", opt.name_);
    }
    fmt::printf("\n");

    for (const auto & opt : opts_) {
        if (!opt.isOptional())
            continue;

        fmt::printf("   %-15s %-15s %-15s\n", opt.name_, opt.has_value_ ? "value" : " ", opt.desc_);
    }
    fmt::printf("   %-15s %-15s %-15s", "-h, -help", " ", "Display this help message\n");

    for (const auto & opt : opts_) {
        if (opt.isOptional())
            continue;

        fmt::printf("   %-15s %-15s %-15s\n", opt.name_, " ", opt.desc_);
    }
}

int TclCmd::ParseArgs(Tcl_Interp* interp, int objc, Tcl_Obj* const* objv)
{
    (void) objc;
    (void) objv;
    (void) interp;

    int n_pos = 0;
    for (auto & opt : opts_) {
        if (!opt.isOptional())
            n_pos++;
        opt.is_set_ = false;
        opt.objv_ = nullptr;
    }

    int i = 1;
    int pos_arg_i = 0;

    while (i < objc) {
        const std::string arg = Tcl_GetString(objv[i]);

        if (arg.length() < 1) {
            fmt::printf("Error: Empty argument\n");
            return TCL_ERROR;
        }

        // Handle help
        if (arg == "-h" || arg == "-help") {
            Help();
            return TCL_OK;
        }

        // Handle switches starting with "-"
        if (arg[0] == '-') {
            bool switch_found = false;

            for (auto & opt : opts_) {

                if (arg != opt.name_)
                    continue;

                switch_found = true;
                opt.is_set_ = true;

                if (!opt.has_value_)
                    continue;

                if (i == objc - 1) {
                    // TODO: Wrap to some logging!
                    fmt::printf("Error: %s is missing value.\n", opt.name_);
                    return TCL_ERROR;
                }

                i++;
                opt.objv_ = objv[i];
            }

            if (switch_found) {
                i++;
                continue;
            }

            fmt::printf("Error: %s is invalid argument to %s. See %s -help for correct usage.\n",
                            arg, name_, name_);
            return TCL_ERROR;
        }

        // Handle positional arguments
        int curr_arg_i = 0;
        bool pos_found = false;

        for (auto & opt : opts_) {
            if (opt.isOptional())
                continue;

            if (curr_arg_i < pos_arg_i) {
                curr_arg_i++;
                continue;
            }

            opt.is_set_ = true;
            opt.objv_ = objv[i];
            pos_arg_i++;
            pos_found = true;
            break;
        }

        if (pos_found) {
            i++;
            continue;
        }

        fmt::printf("Error: Too many positonal arguments: %d. "
                            "See %s -help for correct usage.\n", pos_arg_i + 1, name_);
        return TCL_ERROR;
    }

    if (n_pos != pos_arg_i) {
       fmt::printf("Error: Not enough positional arguments: %d. "
                           "See %s -help for correct usage.\n", pos_arg_i, name_);
        return TCL_ERROR;
    }

    return TCL_OK;
}

}