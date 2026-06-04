
#include "open_char.h"

#include "Context.h"
#include "Algorithms.h"

#include "TestUtils.h"

using namespace open_char;

static void test_inv(Context &ctx, Algorithms &algs)
{
    CREATE_INV_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableLeakageSims(c1));
    algs.MeasureComboLogicTables(c1);

    assert (c1.GetPin("Y").GetLogicTableEntry(0).first == 0);
    assert (c1.GetPin("Y").GetLogicTableEntry(0).second == 1);

    assert (c1.GetPin("Y").GetLogicTableEntry(1).first == 1);
    assert (c1.GetPin("Y").GetLogicTableEntry(1).second == 0);
}

static void test_nand2(Context &ctx, Algorithms &algs)
{
    CREATE_NAND2_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableLeakageSims(c1));
    algs.MeasureComboLogicTables(c1);

    assert (c1.GetPin("Y").GetLogicTableEntry(0).first == 0b00);
    assert (c1.GetPin("Y").GetLogicTableEntry(0).second == 1);

    assert (c1.GetPin("Y").GetLogicTableEntry(1).first == 0b01);
    assert (c1.GetPin("Y").GetLogicTableEntry(1).second == 1);

    assert (c1.GetPin("Y").GetLogicTableEntry(2).first == 0b10);
    assert (c1.GetPin("Y").GetLogicTableEntry(2).second == 1);

    assert (c1.GetPin("Y").GetLogicTableEntry(3).first == 0b11);
    assert (c1.GetPin("Y").GetLogicTableEntry(3).second == 0);
}

static void test_half_adder(Context &ctx, Algorithms &algs)
{
    CREATE_HALF_ADDER_CELL(ctx, c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableLeakageSims(c1));
    assert(algs.MeasureComboLogicTables(c1));

    assert (c1.GetPin("CO").GetLogicTableEntry(0).first == 0b00);
    assert (c1.GetPin("CO").GetLogicTableEntry(0).second == 0);

    assert (c1.GetPin("CO").GetLogicTableEntry(1).first == 0b01);
    assert (c1.GetPin("CO").GetLogicTableEntry(1).second == 0);

    assert (c1.GetPin("CO").GetLogicTableEntry(2).first == 0b10);
    assert (c1.GetPin("CO").GetLogicTableEntry(2).second == 0);

    assert (c1.GetPin("CO").GetLogicTableEntry(3).first == 0b11);
    assert (c1.GetPin("CO").GetLogicTableEntry(3).second == 1);

    assert (c1.GetPin("S").GetLogicTableEntry(0).first == 0b00);
    assert (c1.GetPin("S").GetLogicTableEntry(0).second == 0);

    assert (c1.GetPin("S").GetLogicTableEntry(1).first == 0b01);
    assert (c1.GetPin("S").GetLogicTableEntry(1).second == 1);

    assert (c1.GetPin("S").GetLogicTableEntry(2).first == 0b10);
    assert (c1.GetPin("S").GetLogicTableEntry(2).second == 1);

    assert (c1.GetPin("S").GetLogicTableEntry(3).first == 0b11);
    assert (c1.GetPin("S").GetLogicTableEntry(3).second == 0);
}

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);
    test_nand2      (ctx, algs);
    test_half_adder (ctx, algs);

    return 0;
}