
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

void test_inv(Context &ctx, Algorithms &algs)
{
    CREATE_INV_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareInputCapSims(c1));
    algs.MeasureInputCap(c1);

    // TODO: Add some test where Rise/Fall are truly different
    assert (EQUAL_WITH_TOL(c1.GetPin("A").GetCapacitanceRiseMax(), 0.000319992));
    assert (EQUAL_WITH_TOL(c1.GetPin("A").GetCapacitanceRiseMin(), 0.000200000));
    assert (EQUAL_WITH_TOL(c1.GetPin("A").GetCapacitanceRiseAvg(), 0.000259974));

    assert (EQUAL_WITH_TOL(c1.GetPin("A").GetCapacitanceFallMax(), 0.000320000));
    assert (EQUAL_WITH_TOL(c1.GetPin("A").GetCapacitanceFallMin(), 0.000200006));
    assert (EQUAL_WITH_TOL(c1.GetPin("A").GetCapacitanceFallAvg(), 0.000259974));
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);

    return 0;
}