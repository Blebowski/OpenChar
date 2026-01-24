
#ifndef CONTEXT_H
#define CONTEXT_H

#include <map>

#include <tcl.h>

#include "open_char.h"
#include "Cell.h"
#include "TclCmd.h"

namespace open_char {

class Context {

    private:
        std::map<std::string, Cell> cells_;

    public:
        std::vector<std::pair<TclCmd, Tcl_ObjCmdProc*>> tcl_commands_;
        Tcl_Interp* interp_;

        bool AddCell(std::string name);
        Cell &GetCell(std::string name);
};

}

#endif