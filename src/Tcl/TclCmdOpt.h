
#ifndef TCL_CMD_OPT_H
#define TCL_CMD_OPT_H

#include <string>

#include <tcl.h>

namespace open_char {

class TclCmdOpt {

    public:
        TclCmdOpt();
        TclCmdOpt(const bool has_value, std::string desc);

        std::string name_;
        const bool has_value_;
        const std::string desc_;

        bool isOptional() const;
        bool is_set_;
        const Tcl_Obj* objv_;
};

}

#endif