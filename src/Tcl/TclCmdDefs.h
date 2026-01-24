
#ifndef TCL_CMD_DEFS_H
#define TCL_CMD_DEFS_H

#include "TclCmd.h"

namespace open_char {

    class DefineCell : public TclCmd {
    public:
        DefineCell(Context *ctx);
        int Execute();
    };

}

#endif