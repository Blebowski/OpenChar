
#include "Library.h"
#include "Utils.h"

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

std::map<std::string, Cell>& Library::GetCells()
{
    return cells_;
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
    supplies_[0].SetVddName(name);
    supplies_[0].SetVddVoltage(val);
}

void Library::SetDefaultSupplyVdd(double val)
{
    supplies_[0].SetVddVoltage(val);
}

void Library::SetDefaultSupplyGnd(std::string name, double val)
{
    supplies_[0].SetGndName(name);
    supplies_[0].SetGndVoltage(val);
}

bool Library::HasSupply(std::string vdd_name)
{
    for (auto &sup : supplies_) {
        if (sup.GetVddName() == vdd_name)
            return true;
    }
    return false;
}

Supply& Library::GetSupply(std::string vdd_name)
{
    for (auto &sup : supplies_) {
        if (sup.GetVddName() == vdd_name)
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

void Library::WriteLiberty(const std::string &name)
{
    FILE *f = fopen(name.c_str(), "w");

    TAB_FPRINTF(0, f, "library (%s) {\n", name);

    size_t tab = 1;

    TAB_FPRINTF(tab, f, "technology : cmos ; \n");
    TAB_FPRINTF(tab, f, "delay_mode : table_lookup ; \n");

    op_cond_.WriteLiberty(f, tab);

    for (auto & cell : cells_) {
        cell.second.WriteLiberty(f, tab);
    }

    TAB_FPRINTF(0, f, "} /* end library */\n", name);
}

}
