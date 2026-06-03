
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
    assert (algs.MeasureInputCap(c1));

    CHECK_FLOAT(c1.GetPin("A").GetCapacitanceRiseMax(), 0.276150415);
    CHECK_FLOAT(c1.GetPin("A").GetCapacitanceRiseMin(), 0.075892807);
    CHECK_FLOAT(c1.GetPin("A").GetCapacitanceRiseAvg(), 0.177503686);

    CHECK_FLOAT(c1.GetPin("A").GetCapacitanceFallMax(), 0.277773677);
    CHECK_FLOAT(c1.GetPin("A").GetCapacitanceFallMin(), 0.065065853);
    CHECK_FLOAT(c1.GetPin("A").GetCapacitanceFallAvg(), 0.175257622);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);

    return 0;
}