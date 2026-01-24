
#ifndef TCL_CMD_OPT_H
#define TCL_CMD_OPT_H

#include <string>

namespace open_char {

class TclCmdOpt {

    public:
        TclCmdOpt(const char *name, const bool has_value, const bool is_positional);

        const char *name_;
        const bool has_value_;
        const bool is_positional_;

        std::string value_;
};

}

#endif