
#include <string>

#include "TclCmdOpt.h"

namespace open_char {

TclCmdOpt::TclCmdOpt(const char *name, const bool has_value, const bool is_positional) :
    name_(name),
    has_value_(has_value),
    is_positional_(is_positional)
{}

}