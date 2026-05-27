
#include "StackTrace.h"
#include "Utils.h"

#define RUN_SIMULATIONS(ctx, prep_func)                                                         \
        do {                                                                                    \
            ctx.GetSimulationPool().Start();                                                    \
            prep_func;                                                                          \
            ctx.GetSimulationPool().WaitDone();                                                 \
        } while (0)

#define ALG_TEST_INIT(ctx, algs)                                                                \
        StackTraceInit();                                                                       \
        Context ctx(nullptr);                                                                   \
        Algorithms algs(&ctx);

#define CREATE_INV_CELL(ctx, name)                                                              \
        ctx.GetLibrary().AddCell("INV");                                                        \
        Cell &c1 = ctx.GetLibrary().GetCell("INV");                                             \
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("Y",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

#define CREATE_AND2_CELL(ctx, name)                                                             \
        ctx.GetLibrary().AddCell("AND2");                                                       \
        Cell &c1 = ctx.GetLibrary().GetCell("AND2");                                            \
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("Z",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

#define CREATE_NAND2_CELL(ctx, name)                                                            \
        ctx.GetLibrary().AddCell("NAND2");                                                      \
        Cell &c1 = ctx.GetLibrary().GetCell("NAND2");                                           \
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("Y",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

#define CREATE_HALF_ADDER_CELL(ctx, name)                                                       \
        ctx.GetLibrary().AddCell("HALF_ADDER");                                                 \
        Cell &c1 = ctx.GetLibrary().GetCell("HALF_ADDER");                                      \
        c1.AddPin("CO",  PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("S",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);
