
#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>

#include <tcl.h>

#include "open_char.h"
#include "Cell.h"

namespace open_char {

class Context {

    public:
        std::vector<Cell> cells_;
        Tcl_Interp* interp_;
};

}

#endif