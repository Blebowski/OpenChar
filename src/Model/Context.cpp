

#include "Context.h"

namespace open_char {

bool Context::AddCell(std::string name)
{
    auto [it, inserted] = cells_.emplace(name, Cell(name));

    if (!inserted)
        return false;

    return true;
}

}