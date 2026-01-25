
#include <string>

#include "TclCmdOpt.h"

namespace open_char {

TclCmdOpt::TclCmdOpt() :
    name_(""),
    has_value_(false),
    desc_(""),
    objv_(nullptr)
{}

TclCmdOpt::TclCmdOpt(const bool has_value, std::string desc) :
    name_(""),
    has_value_(has_value),
    desc_(desc),
    objv_(nullptr)
{}

bool TclCmdOpt::isOptional() const
{
    if (name_[0] == '-')
        return true;
    return false;
}

}