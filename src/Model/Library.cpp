
#include "Library.h"

namespace open_char {

Library::Library() :
    supplies_({{"VDD", 1.2, "VSS", 0}}),
    op_cond_(&supplies_[0])
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

OpCond& Library::GetOpCond()
{
    return op_cond_;
}

void Library::SetDefaultSupplyVdd(std::string name, double val)
{
    supplies_[0].vdd_name_ = name;
    supplies_[0].vdd_val_ = val;
}

void Library::SetDefaultSupplyVdd(double val)
{
    supplies_[0].vdd_val_ = val;
}

void Library::SetDefaultSupplyGnd(std::string name, double val)
{
    supplies_[0].gnd_name_ = name;
    supplies_[0].gnd_val_ = val;
}

bool Library::HasSupply(std::string vdd_name)
{
    for (const auto &sup : supplies_) {
        if (sup.vdd_name_ == vdd_name)
            return true;
    }
    return false;
}

Supply& Library::GetSupply(std::string vdd_name)
{
    for (auto &sup : supplies_) {
        if (sup.vdd_name_ == vdd_name)
            return sup;
    }
    return supplies_[0];
}

std::pair<Template&, bool> Library::AddTemplate(std::string name)
{
    auto [it, inserted] = templates_.emplace(name, Template(name));
    return {it->second, inserted};
}

bool Library::HasTemplate(std::string name)
{
    if (templates_.contains(name))
        return true;
    return false;
}

Template& Library::GetTemplate(std::string name)
{
    return templates_.find(name)->second;
}

}
