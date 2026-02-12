
#include <cassert>
#include <filesystem>

#include "Supply.h"
#include "Utils.h"

using namespace open_char;

int main()
{
    Supply *s1 = new Supply("VDDIO", 0.7, "VSSIO", 0.0);

    assert(s1->GetVddName()      == "VDDIO");
    assert(s1->GetGndName()      == "VSSIO");
    assert(COMPARE_FLOATS(s1->GetVddVoltage(), 0.7));
    assert(COMPARE_FLOATS(s1->GetGndVoltage(), 0.0));

    std::string vss = "VSS";
    std::string vdd = "VDD";
    s1->SetGndName(vss);
    s1->SetVddName(vdd);
    s1->SetGndVoltage(0.5);
    s1->SetVddVoltage(1.9);

    assert (s1->GetGndName() == "VSS");
    assert (s1->GetVddName() == "VDD");
    assert(COMPARE_FLOATS(s1->GetVddVoltage(), 1.9));
    assert(COMPARE_FLOATS(s1->GetGndVoltage(), 0.5));

    delete s1;
}