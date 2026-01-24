
#ifndef TCL_CMD_DEFS_H
#define TCL_CMD_DEFS_H

#include "TclCmd.h"

namespace open_char {

#define CREATE_TCL_COMMAND(class_name, cmd_name, params, exec_rtn)                                  \
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
            params                                                                                  \
        ) {}                                                                                        \
                                                                                                    \
    int class_name::Execute()                                                                       \
    {                                                                                               \
        exec_rtn                                                                                    \
        return TCL_OK;                                                                              \
    }                                                                                               \
                                                                                                    \
    int class_name##Cb(void *data, Tcl_Interp* interp, int objc, Tcl_Obj* const* objv)              \
    {                                                                                               \
        class_name *cmd = (class_name *) data;                                                      \
                                                                                                    \
        cmd->ParseArgs(interp, objc, objv);                                                         \
        return cmd->Execute();                                                                      \
    }                                                                                               \

}

#endif