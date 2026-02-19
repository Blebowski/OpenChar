
#include "open_char.h"
#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include <cassert>

using namespace open_char;

void test_inv(Context &ctx, Algorithms &algs)
{
    ctx.GetLibrary().AddCell("INV");
    Cell &c1 = ctx.GetLibrary().GetCell("INV");

    Template t("MY_TEMP");
    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    c1.AddPin("Y",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

    algs.PrepareLogicTableAndLeakageSims(c1);

    ctx.GetSimulationPool().StartSimulations();
    ctx.GetSimulationPool().FinishAndProcessSimulations();

    algs.CalculateLogicFunctions(c1);

    algs.PrepareComboDelaySims(c1);
    ctx.GetSimulationPool().StartSimulations();
    ctx.GetSimulationPool().FinishAndProcessSimulations();

    Pin& pin = c1.GetPin("Y");

    TimingArc& arc = pin.GetTimingArcs()[0];

    // Default numbers for basic NMOS and PMOS in NGSPICE
    assert (arc.GetFallDelays()[0][0] > 0.18 && arc.GetFallDelays()[0][0] < 0.19);
    assert (arc.GetFallDelays()[0][1] > 2.72 && arc.GetFallDelays()[0][1] < 2.73);
    assert (arc.GetFallDelays()[1][0] > 0.20 && arc.GetFallDelays()[1][0] < 0.21);
    assert (arc.GetFallDelays()[1][1] > 2.75 && arc.GetFallDelays()[1][1] < 2.76);

    assert (arc.GetRiseDelays()[0][0] > 0.09 && arc.GetRiseDelays()[0][0] < 0.10);
    assert (arc.GetRiseDelays()[0][1] > 1.36 && arc.GetRiseDelays()[0][1] < 1.37);
    assert (arc.GetRiseDelays()[1][0] > 0.11 && arc.GetRiseDelays()[1][0] < 0.12);
    assert (arc.GetRiseDelays()[1][1] > 1.38 && arc.GetRiseDelays()[1][1] < 1.39);

    /*
    Inputs from: 00000000
    Inputs to:   00000001
    Output from:        1
    Output to:          0
    ---------------------
    | 0.18410 | 2.72870 |
    | 0.20790 | 2.75010 |
    ---------------------
    ---------------------
    Inputs from: 00000001
    Inputs to:   00000000
    Output from:        0
    Output to:          1
    ---------------------
    | 0.09330 | 1.36650 |
    | 0.11510 | 1.38750 |
    ---------------------
    */

}

int main()
{
    Context ctx(nullptr);
    Algorithms algs(&ctx);

    ctx.AddNetlist(TEST_COMMON_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);

    return 0;
}