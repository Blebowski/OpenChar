
#include "open_char.h"
#include "Context.h"
#include "Algorithms.h"

#include <cassert>

int main()
{
    using namespace open_char;

    Context ctx;
    Algorithms algs(&ctx);

    ctx.includes_.push_back(TEST_COMMON_DIR "/basic_gates.cdl");

    ctx.lib_.AddCell("INV");
    Cell &c1 = ctx.lib_.GetCell("INV");

    c1.AddPin("Y",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

    algs.GetLogicFunction(c1);

    assert (c1.GetPins()["Y"].GetLogicTableEntry(0).first == 0);
    assert (c1.GetPins()["Y"].GetLogicTableEntry(0).second == 1);

    assert (c1.GetPins()["Y"].GetLogicTableEntry(1).first == 1);
    assert (c1.GetPins()["Y"].GetLogicTableEntry(1).second == 0);

    return 0;
}