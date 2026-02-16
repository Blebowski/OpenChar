
#include <algorithm>

#include "open_char.h"
#include "Cell.h"
#include "Utils.h"

namespace open_char {

Cell::Cell(std::string name, Library *library) :
    name_(name),
    library_(library)
{};

Cell::~Cell()
{
    for (const auto & lkg : leakage_table_)
        delete lkg.first;
}

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

void Cell::AddLeakageTableEntry(Expression *e, NanoWatt pwr)
{
    leakage_table_.push_back(std::make_pair(e, pwr));
}

void Cell::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "cell (%s) {\n", name_);
    tab++;

    TAB_FPRINTF(tab, f, "area: 0.00\n");

    for (auto & lkg : leakage_table_) {
        TAB_FPRINTF(tab, f, "leakage_power () {\n");
        tab++;
        TAB_FPRINTF(tab, f, "value : %f;\n", lkg.second);
        TAB_FPRINTF(tab, f, "when : \"");
        lkg.first->Print(f);
        fprintf(f, "\";\n");
        tab--;
        TAB_FPRINTF(tab, f, "} /* end leakage_power */\n");
    }

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