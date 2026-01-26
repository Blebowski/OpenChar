

#include "Context.h"

namespace open_char {

Context::Context()
{
    algorithms_ = new Algorithms(this);
}

Context::~Context()
{
    delete algorithms_;
}

std::pair<Cell&, bool> Context::AddCell(std::string name)
{
    auto [it, inserted] = cells_.emplace(name, Cell(name));
    return {it->second, inserted};
}

Cell& Context::GetCell(std::string name)
{
    return cells_.find(name)->second;
}

bool Context::HasCell(std::string name)
{
    if (cells_.contains(name))
        return true;
    return false;
}

}