////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

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