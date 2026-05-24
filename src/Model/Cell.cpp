
#include <algorithm>

#include "open_char.h"
#include "Cell.h"
#include "Utils.h"
#include "Simulation.h"

namespace open_char {

Cell::Cell(std::string name, Library *library) :
    name_(name),
    library_(library),
    seq_(this),
    charact_state_(CharactState::START)
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

CellKind Cell::GetKind()
{
    return kind_;
}

void Cell::SetKind(CellKind kind)
{
    kind_ = kind;
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

Sequential& Cell::GetSequential()
{
    return seq_;
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

void Cell::AddSimulation(Simulation *simulation)
{
    simulations_.push_back(simulation);
}

std::vector<Simulation*>& Cell::GetSimulations()
{
    return simulations_;
}

bool Cell::IsSimulationFinished()
{
    // TODO: When there are many simulations in cell, its pins and arcs, checking
    //       if simulation is finished is expensive as each check in Simulation
    //       object involves mutex lock / unlock. Later this may need to be optimized

    for (auto & sim : simulations_) {
        if (!sim->IsFinished())
            return false;
    }

    for (auto & pin : pins_) {
        for (auto & sim : pin.second.GetSimulations()) {
            if (!sim->IsFinished())
                return false;
        }
        for (auto & arc : pin.second.GetArcs()) {
            for (auto & sim : arc.GetSimulations()) {
                if (!sim->IsFinished())
                    return false;
            }
        }
    }

    return true;
}

void Cell::SetCharactState(CharactState charact_state)
{
    charact_state_ = charact_state;
}

CharactState Cell::GetCharactState()
{
    return charact_state_;
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

    if (kind_ == CellKind::SEQUENTIAL) {
        seq_.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::PWR)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::GND)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::CLK)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::ASYNC)) {
        pin.WriteLiberty(f, tab);
    }

    for (auto & pin : GetPins(PinKind::DATA)) {
        pin.WriteLiberty(f, tab);
    }

    tab--;
    TAB_FPRINTF(tab, f, "} /* end cell */\n");
}

}