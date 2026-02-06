
#ifndef TCL_CMD_DEFS_H
#define TCL_CMD_DEFS_H

#include "TclCmd.h"
#include "Utils.h"

namespace open_char {

#define ARG(...) __VA_ARGS__

#define CREATE_TCL_COMMAND(class_name, cmd_name, desc, print_ret_code, params, exec_rtn)            \
                                                                                                    \
    class class_name : public TclCmd {                                                              \
    public:                                                                                         \
        class_name(Context *ctx);                                                                   \
        int Execute();                                                                              \
    };                                                                                              \
                                                                                                    \
    class_name::class_name(Context *ctx):                                                           \
        TclCmd (                                                                                    \
            ctx,                                                                                    \
            cmd_name,                                                                               \
            desc,                                                                                   \
            print_ret_code,                                                                         \
            params                                                                                  \
        ) {}                                                                                        \
                                                                                                    \
    int class_name::Execute()                                                                       \
    {                                                                                               \
        exec_rtn                                                                                    \
    }                                                                                               \
                                                                                                    \
    int class_name##Cb(void *data, Tcl_Interp* interp, int objc, Tcl_Obj* const* objv)              \
    {                                                                                               \
        class_name *cmd = (class_name *) data;                                                      \
        int parse_rv = cmd->ParseArgs(interp, objc, objv);                                          \
        if (parse_rv == -1)                                                                         \
            return TCL_OK;                                                                          \
        if (parse_rv != TCL_OK)                                                                     \
            return parse_rv;                                                                        \
        int rv = cmd->Execute();                                                                    \
        if (cmd->print_ret_code_) {                                                                 \
            if (rv == TCL_OK)                                                                       \
                printf("1\n");                                                                      \
            else                                                                                    \
                printf("0\n");                                                                      \
        }                                                                                           \
        return rv;                                                                                  \
    }                                                                                               \

}

#endif