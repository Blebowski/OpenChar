
#include "open_char.h"
#include "Cell.h"

namespace open_char {

Cell::Cell(std::string name) :
    name_(name)
{};

std::pair<Pin&, bool> Cell::AddPin(std::string name, PinDirection direction, PinKind kind)
{
    auto [it, inserted] = pins_.emplace(name, Pin(this, name, direction, kind));
    return {it->second, inserted};
}

std::map<std::string, Pin>& Cell::GetPins()
{
    return pins_;
}

size_t Cell::GetPinsCount(PinDirection direction)
{
    return std::count_if(pins_.cbegin(), pins_.cend(),
                        [direction](const auto & pin){
                            return pin.second.direction_ == direction;
                        });
}

}