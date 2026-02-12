
#include <tcl.h>

namespace open_char {

#define CREATE_TCL_VAR_CB(var_name, prop_name)                                      \
    static char *prop_name##Cb(void *clientData, Tcl_Interp *interp,                \
                        const char *name1, const char *name2, int flags)            \
    {                                                                               \
        Context* ctx = static_cast<Context*>(clientData);                           \
        std::string val = Tcl_GetVar(ctx->GetTclInterp(), var_name, 0);             \
        ctx->GetVariables().Set##prop_name(val);                                    \
        return NULL;                                                                \
    }

#define REGISTER_TCL_VAR_CB(var_name, prop_name)                                            \
    {                                                                                       \
        Tcl_TraceVar(interp, var_name, TCL_TRACE_WRITES, prop_name##Cb,                     \
                     static_cast<void*>(ctx));                                              \
        Tcl_SetVar(interp, var_name, ctx->GetVariables().Get ##prop_name().c_str(), 0);     \
    }

}