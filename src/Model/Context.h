
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

        // Global environment
        double temp_ = 25;

        std::pair<std::string, double> vcc_ = {"VDD", 1.2};
        std::pair<std::string, double> vss_ = {"VSS", 0};

        std::vector<std::string> libs_ = {"models.lib tt"};
        std::vector<std::string> includes_ = {"cells.cdl"};

        // TCL objects
        std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>> tcl_commands_;
        Tcl_Interp* interp_;

        // Other
        Algorithms *algorithms_;
};

}

#endif