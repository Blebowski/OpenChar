
#include "OpCond.h"
#include "Utils.h"

namespace open_char {

OpCond::OpCond(Supply *supply) :
    temp_(25.0),
    supply_(supply)
{}

void OpCond::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "operating conditions (%s) {\n", name_);

    tab++;
    TAB_FPRINTF(tab, f, "process : 1.00 ;\n");
    TAB_FPRINTF(tab, f, "voltage : %f ;\n", supply_->GetVddVoltage());
    TAB_FPRINTF(tab, f, "temperature : %f ;\n", temp_);
    tab--;

    TAB_FPRINTF(tab, f, "} /* end operating_conditions */\n");
}

}