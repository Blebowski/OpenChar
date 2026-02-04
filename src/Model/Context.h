
#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>

#include <tcl.h>

#include "open_char.h"
#include "Cell.h"
#include "TclCmd.h"
#include "Algorithms.h"
#include "Library.h"

namespace open_char {

class Context {

    public:

        Context();
        ~Context();

        // Cell library to characterize
        Library lib_;

        std::vector<std::string> libs_;
        std::vector<std::string> includes_;

        // TCL objects
        std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>> tcl_commands_;
        Tcl_Interp* interp_;

        // Other
        Algorithms *algorithms_;
};

}

#endif