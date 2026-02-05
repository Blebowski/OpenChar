
#include <string>

#include "TclCmdOpt.h"

namespace open_char {

TclCmdOpt::TclCmdOpt() :
    name_(""),
    has_value_(false),
    value_desc_(""),
    desc_(""),
    objv_(nullptr)
{}

TclCmdOpt::TclCmdOpt(const bool has_value, std::string value_desc, std::string desc) :
    name_(""),
    has_value_(has_value),
    value_desc_(value_desc),
    desc_(desc),
    objv_(nullptr)
{}

bool TclCmdOpt::isOptional() const
{
    if (name_[0] == '-')
        return true;
    return false;
}

bool TclCmdOpt::isSet() const
{
    if (objv_ == nullptr)
        return false;
    return true;
}

}