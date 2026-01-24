
#ifndef TCL_CMD_H
#define TCL_CMD_H

#include <string>
#include <vector>

#include <tcl.h>

#include "Context.h"
#include "TclCmdOpt.h"

namespace open_char {

class TclCmd {

    public:
        TclCmd(Context *ctx, std::string name, std::vector<TclCmdOpt> opts);

        Context *ctx_;
        const std::string name_;
        std::vector<TclCmdOpt> opts_;

        int ParseArgs(Tcl_Interp* interp, int objc, Tcl_Obj* const* objv);
};

}

#endif