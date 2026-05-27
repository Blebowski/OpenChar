
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

void test_inv(Context &ctx, Algorithms &algs)
{
    CREATE_INV_CELL(ctx, c1)

    Template t("MY_TEMP");
    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    // Logic table and logic function are precondition for combo delays
    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableAndLeakageSims(c1));
    algs.MeasureComboLogicTables(c1);
    algs.CalculateComboLogicFunctions(c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComboDelayAndPowerSims(c1));
    algs.MeasureComboDelays(c1);

    Pin& pin = c1.GetPin("Y");

    Arc& arc = pin.GetArcs()[0];

    // Default numbers for basic NMOS and PMOS in NGSPICE
    assert (arc.GetFallDelays()[0][0] > 0.18 && arc.GetFallDelays()[0][0] < 0.19);
    assert (arc.GetFallDelays()[0][1] > 2.72 && arc.GetFallDelays()[0][1] < 2.73);
    assert (arc.GetFallDelays()[1][0] > 0.20 && arc.GetFallDelays()[1][0] < 0.21);
    assert (arc.GetFallDelays()[1][1] > 2.75 && arc.GetFallDelays()[1][1] < 2.76);

    assert (arc.GetRiseDelays()[0][0] > 0.09 && arc.GetRiseDelays()[0][0] < 0.10);
    assert (arc.GetRiseDelays()[0][1] > 1.36 && arc.GetRiseDelays()[0][1] < 1.37);
    assert (arc.GetRiseDelays()[1][0] > 0.11 && arc.GetRiseDelays()[1][0] < 0.12);
    assert (arc.GetRiseDelays()[1][1] > 1.38 && arc.GetRiseDelays()[1][1] < 1.39);

    // Inputs from: 00000000
    // Inputs to:   00000001
    // Output from:        1
    // Output to:          0
    // ---------------------
    // | 0.18410 | 2.72870 |
    // | 0.20790 | 2.75010 |
    // ---------------------
    // ---------------------
    // Inputs from: 00000001
    // Inputs to:   00000000
    // Output from:        0
    // Output to:          1
    // ---------------------
    // | 0.09330 | 1.36650 |
    // | 0.11510 | 1.38750 |
    // ---------------------
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);

    return 0;
}