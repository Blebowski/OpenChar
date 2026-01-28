
#include "Library.h"

namespace open_char {

Library::Library()
{}

std::pair<Cell&, bool> Library::AddCell(std::string name)
{
    auto [it, inserted] = cells_.emplace(name, Cell(name));
    return {it->second, inserted};
}

Cell& Library::GetCell(std::string name)
{
    return cells_.find(name)->second;
}

bool Library::HasCell(std::string name)
{
    if (cells_.contains(name))
        return true;
    return false;
}

}
