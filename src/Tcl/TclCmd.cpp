
#include "TclCmd.h"
#include "Utils.h"

namespace open_char {

TclCmd::TclCmd(Context *ctx, std::string name, std::string desc,
                std::map<std::string, TclCmdOpt> opts) :
    ctx_(ctx),
    name_(name),
    desc_(desc),
    opts_(opts)
{
    // Copy the string intentionally to keep it simple
    for (auto &x : opts_)
        x.second.name_ = x.first;
}

void TclCmd::Help(void)
{
    printf("%s\n\n", desc_);

    bool has_opts = false;
    for (const auto & opt_p : opts_) {
        if (opt_p.second.isOptional()) {
            has_opts = true;
            break;
        }
    }

    printf("%s %s", name_, (has_opts) ? "[options]" : "");
    for (const auto & opt_p : opts_) {
        auto &opt = opt_p.second;

        if (opt.isOptional())
            continue;

        printf(" %s", opt.name_);
    }
    printf("\n");

    for (const auto & opt_p : opts_) {
        auto &opt = opt_p.second;

        if (!opt.isOptional())
            continue;

        printf("   %-15s %-15s %-15s\n", opt.name_,
                opt.has_value_ ? "value" : " ", opt.desc_);
    }
    printf("   %-15s %-15s %-15s", "-h, -help", " ", "Display this help message\n");

    for (const auto & opt_p : opts_) {
        if (opt_p.second.isOptional())
            continue;

        printf("   %-15s %-15s %-15s\n", opt_p.second.name_, " ", opt_p.second.desc_);
    }
}

int TclCmd::ParseArgs(Tcl_Interp* interp, int objc, Tcl_Obj* const* objv)
{
    (void) objc;
    (void) objv;
    (void) interp;

    int n_pos = 0;
    for (auto & opt_p : opts_) {
        auto &opt = opt_p.second;

        if (!opt.isOptional())
            n_pos++;

        opt.objv_ = nullptr;
    }

    int i = 1;
    int pos_arg_i = 0;

    while (i < objc) {
        const std::string arg = Tcl_GetString(objv[i]);

        if (arg.length() < 1) {
            printf("Error: Empty argument\n");
            return TCL_ERROR;
        }

        // Handle help
        if (arg == "-h" || arg == "-help") {
            Help();
            return -1;
        }

        // Handle switches starting with "-"
        if (arg[0] == '-') {
            bool switch_found = false;

            for (auto & opt_p : opts_) {

                auto &opt = opt_p.second;

                if (arg != opt.name_)
                    continue;

                switch_found = true;

                if (!opt.has_value_)
                    continue;

                if (i == objc - 1) {
                    // TODO: Wrap to some logging!
                    printf("Error: %s is missing value.\n", opt.name_);
                    return TCL_ERROR;
                }

                i++;
                opt.objv_ = objv[i];
            }

            if (switch_found) {
                i++;
                continue;
            }

            printf("Error: %s is invalid argument to %s. See %s -help for correct usage.\n",
                    arg, name_, name_);
            return TCL_ERROR;
        }

        // Handle positional arguments
        int curr_arg_i = 0;
        bool pos_found = false;

        for (auto & opt_p : opts_) {
            auto &opt = opt_p.second;

            if (opt.isOptional())
                continue;

            if (curr_arg_i < pos_arg_i) {
                curr_arg_i++;
                continue;
            }

            opt.objv_ = objv[i];
            pos_arg_i++;
            pos_found = true;
            break;
        }

        if (pos_found) {
            i++;
            continue;
        }

        printf("Error: Too many positonal arguments: %d. "
                "See %s -help for correct usage.\n", pos_arg_i + 1, name_);
        return TCL_ERROR;
    }

    if (n_pos != pos_arg_i) {
       printf("Error: Not enough positional arguments: %d. "
               "See %s -help for correct usage.\n", pos_arg_i, name_);
        return TCL_ERROR;
    }

    return TCL_OK;
}

}