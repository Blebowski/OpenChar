
#include "open_char.h"

#include "Context.h"
#include "Algorithms.h"

#include "TestUtils.h"

using namespace open_char;

void test_inv(Context &ctx, Algorithms &algs)
{
    ctx.GetLibrary().AddCell("INV");
    Cell &c1 = ctx.GetLibrary().GetCell("INV");

    c1.AddPin("Y",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableAndLeakageSims(c1));
    algs.MeasureComboLogicTables(c1);

    assert (c1.GetPin("Y").GetLogicTableEntry(0).first == 0);
    assert (c1.GetPin("Y").GetLogicTableEntry(0).second == 1);

    assert (c1.GetPin("Y").GetLogicTableEntry(1).first == 1);
    assert (c1.GetPin("Y").GetLogicTableEntry(1).second == 0);
}

void test_nand2(Context &ctx, Algorithms &algs)
{
    ctx.GetLibrary().AddCell("NAND2");
    Cell &c1 = ctx.GetLibrary().GetCell("NAND2");

    c1.AddPin("Y",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableAndLeakageSims(c1));
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

void test_half_adder(Context &ctx, Algorithms &algs)
{
    ctx.GetLibrary().AddCell("HALF_ADDER");
    Cell &c1 = ctx.GetLibrary().GetCell("HALF_ADDER");

    c1.AddPin("CO",  PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("S",   PinDirection::OUT,     PinKind::DATA);
    c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);
    c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);
    c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableAndLeakageSims(c1));
    algs.MeasureComboLogicTables(c1);

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