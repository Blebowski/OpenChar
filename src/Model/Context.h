
#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>

#include <tcl.h>

#include "open_char.h"
#include "Cell.h"
#include "TclCmd.h"

namespace open_char {

class Context {

    public:
        std::vector<Cell> cells_;

        std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>> tcl_commands_;
        Tcl_Interp* interp_;
};

}

#endif