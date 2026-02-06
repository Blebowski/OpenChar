
#ifndef TCL_CMD_H
#define TCL_CMD_H

#include <string>
#include <map>

#include <tcl.h>

#include "Context.h"
#include "TclCmdOpt.h"

namespace open_char {

class TclCmd {

    public:
        TclCmd(Context *ctx, std::string name, std::string desc, bool print_ret_code,
               std::map<std::string, TclCmdOpt> opts);

        Context *ctx_;
        const std::string name_;
        const std::string desc_;
        const bool print_ret_code_;
        std::map<std::string, TclCmdOpt> opts_;

        int ParseArgs(Tcl_Interp* interp, int objc, Tcl_Obj* const* objv);
        void Help(void);
};

}

#endif