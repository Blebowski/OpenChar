
#include <string>

#include "TclCmdOpt.h"

namespace open_char {

TclCmdOpt::TclCmdOpt(const char* name, const bool has_value, const char* desc) :
    name_(name),
    has_value_(has_value),
    desc_(desc),
    is_set_(false),
    objv_(nullptr)
{}

bool TclCmdOpt::isOptional() const
{
    if (name_[0] == '-')
        return true;
    return false;
}

}