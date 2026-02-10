
#include <algorithm>

#include "open_char.h"
#include "Cell.h"
#include "Utils.h"

namespace open_char {

Cell::Cell(std::string name, Library *library) :
    name_(name),
    library_(library)
{};

Library* Cell::GetLibrary()
{
    assert(library_ != nullptr);
    return library_;
}

const std::string& Cell::GetName()
{
    return name_;
}

std::pair<Pin&, bool> Cell::AddPin(std::string name, PinDirection direction, PinKind kind)
{
    auto [it, inserted] = pins_.emplace(name, Pin(this, name, direction, kind));
    return {it->second, inserted};
}

Pin& Cell::GetPin(std::string name)
{
    return pins_[name];
}

size_t Cell::GetPinsCount(PinDirection direction)
{
    return std::count_if(pins_.cbegin(), pins_.cend(),
                        [direction](const auto & pin){
                            return pin.second.direction_ == direction;
                        });
}

Template* Cell::GetDelayTemplate()
{
    return d_template_;
}

void Cell::SetDelayTemplate(Template *d_template)
{
    d_template_ = d_template;
}

void Cell::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "cell (%s) {\n", name_);
    tab++;

    TAB_FPRINTF(tab, f, "area: 0.00\n");

    for (auto & pin : GetPins(PinKind::PWR)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::GND)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::DATA)) {
        pin.WriteLiberty(f, tab);
    }

    tab--;
    TAB_FPRINTF(tab, f, "} /* end cell */\n");
}

}