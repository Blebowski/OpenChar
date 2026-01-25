
#include "open_char.h"
#include "Cell.h"

namespace open_char {

Cell::Cell(std::string name) :
    name_(name)
{};

std::pair<Pin&, bool> Cell::AddPin(std::string name, PinDirection direction, PinKind kind)
{
    auto [it, inserted] = pins_.emplace(name, Pin(name, direction, kind));
    return {it->second, inserted};
}

}
