
#include "OpCond.h"
#include "Utils.h"

namespace open_char {

OpCond::OpCond(Supply *supply) :
    name_("DEFAULT_OPCOND"),
    supply_(supply),
    temperature_(25.0)
{}

void OpCond::SetName(std::string name)
{
    name_ = name;
}

const std::string& OpCond::GetName()
{
    return name_;
}

void OpCond::SetTemperature(Celsius temperature)
{
    temperature_ = temperature;
}

Celsius OpCond::GetTemperature()
{
    return temperature_;
}

Supply* OpCond::GetSupply()
{
    assert (supply_ != nullptr);
    return supply_;
}

void OpCond::SetSupply(Supply *supply)
{
    supply_ = supply;
}

void OpCond::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "operating conditions (%s) {\n", name_);

    tab++;
    TAB_FPRINTF(tab, f, "process : 1.00 ;\n");
    TAB_FPRINTF(tab, f, "voltage : %f ;\n", supply_->GetVddVoltage());
    TAB_FPRINTF(tab, f, "temperature : %f ;\n", temperature_);
    tab--;

    TAB_FPRINTF(tab, f, "} /* end operating_conditions */\n");
}

}