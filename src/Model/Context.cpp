

#include "Context.h"

namespace open_char {

std::pair<Cell&, bool> Context::AddCell(std::string name)
{
    auto [it, inserted] = cells_.emplace(name, Cell(name));
    return {it->second, inserted};
}

}