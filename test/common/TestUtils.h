
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
        ctx.GetSimulationPool().SetNumThreads(10);                                              \
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
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("CO",  PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("S",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("B",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

#define CREATE_DFF_CKB_SB_RB_CELL(ctx, name)                                                    \
        ctx.GetLibrary().AddCell("DFF_CKB_SB_RB");                                              \
        Cell &c1 = ctx.GetLibrary().GetCell("DFF_CKB_SB_RB");                                   \
        c1.SetKind(CellKind::SEQUENTIAL);                                                       \
        c1.AddPin("Q",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("CKB", PinDirection::IN,      PinKind::CLK);                                  \
        c1.AddPin("D",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("RB",  PinDirection::IN,      PinKind::ASYNC);                                \
        c1.AddPin("SB",  PinDirection::IN,      PinKind::ASYNC);                                \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

#define CREATE_DFF_CK_SB_RB_CELL(ctx, name)                                                     \
        ctx.GetLibrary().AddCell("DFF_CK_SB_RB");                                               \
        Cell &c1 = ctx.GetLibrary().GetCell("DFF_CK_SB_RB");                                    \
        c1.SetKind(CellKind::SEQUENTIAL);                                                       \
        c1.AddPin("Q",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("CK",  PinDirection::IN,      PinKind::CLK);                                  \
        c1.AddPin("D",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("RB",  PinDirection::IN,      PinKind::ASYNC);                                \
        c1.AddPin("SB",  PinDirection::IN,      PinKind::ASYNC);                                \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

#define CREATE_LAT_CKB_R_S_CELL(ctx, name)                                                      \
        ctx.GetLibrary().AddCell("LAT_CKB_R_S");                                                \
        Cell &c1 = ctx.GetLibrary().GetCell("LAT_CKB_R_S");                                     \
        c1.SetKind(CellKind::SEQUENTIAL);                                                       \
        c1.AddPin("Q",   PinDirection::OUT,     PinKind::DATA);                                 \
        c1.AddPin("D",   PinDirection::IN,      PinKind::DATA);                                 \
        c1.AddPin("GB",  PinDirection::IN,      PinKind::CLK);                                  \
        c1.AddPin("RB",  PinDirection::IN,      PinKind::ASYNC);                                \
        c1.AddPin("SB",  PinDirection::IN,      PinKind::ASYNC);                                \
        c1.AddPin("VDD", PinDirection::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDirection::INOUT,   PinKind::PWR);

// For now we set 1 % tolerance
// This may need to be updated if we make time-step configurable
#define EQUAL_WITH_TOL(a, b) (std::fabs(static_cast<double>(a)) - std::fabs(static_cast<double>(b)) < std::fabs((static_cast<double>(a)) * 0.01))
