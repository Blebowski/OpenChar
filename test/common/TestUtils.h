
#include <filesystem>

#include "StackTrace.h"
#include "Utils.h"

#define TEST_RUN_DIR "characterization_data"

#define RUN_SIMULATIONS(ctx, prep_func)                                                         \
        do {                                                                                    \
            ctx.GetSimulationPool().Start();                                                    \
            prep_func;                                                                          \
            ctx.GetSimulationPool().WaitDone();                                                 \
        } while (0)

#define ALG_TEST_INIT(ctx, algs)                                                                \
        StackTraceInit();                                                                       \
        Context ctx(nullptr);                                                                   \
        ctx.GetVariables().SetVariable("run_directory", TEST_RUN_DIR);                          \
        std::filesystem::remove_all(TEST_RUN_DIR);                                              \
        ctx.GetSimulationPool().SetNumThreads(10);                                              \
        Algorithms algs(&ctx);

// For now we set 2.5 % tolerance
// This may need to be updated if we make time-step configurable
#define FABS(x) std::fabs(static_cast<double>(x))
#define TOLERANCE 0.025
#define EQUAL_WITH_TOL(obs, exp) ((FABS(obs - exp)) < (FABS(exp) * TOLERANCE))

#define CHECK_FLOAT(obs, exp)                                                                   \
            do {                                                                                \
                if (!EQUAL_WITH_TOL(obs, exp)) {                                                \
                    info("Observed:  %f",  obs);                                                \
                    info("Expected:  %f +- %f (%f - %f)",  exp, exp * TOLERANCE,                \
                         exp - exp * TOLERANCE, exp + exp * TOLERANCE);                         \
                    assert(false && "Expected does not match observed!");                       \
                }                                                                               \
            } while (0)


/////////////////////////////////////////////////////////////////////////////////////////////////
// Cell creation wrappers
/////////////////////////////////////////////////////////////////////////////////////////////////

#define CREATE_INV_CELL(ctx, name)                                                              \
        ctx.GetLibrary().AddCell("INV");                                                        \
        Cell &c1 = ctx.GetLibrary().GetCell("INV");                                             \
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("Y",   PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDir::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDir::INOUT,   PinKind::PWR);

#define CREATE_AND2_CELL(ctx, name)                                                             \
        ctx.GetLibrary().AddCell("AND2");                                                       \
        Cell &c1 = ctx.GetLibrary().GetCell("AND2");                                            \
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("Z",   PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("B",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDir::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDir::INOUT,   PinKind::PWR);

#define CREATE_NAND2_CELL(ctx, name)                                                            \
        ctx.GetLibrary().AddCell("NAND2");                                                      \
        Cell &c1 = ctx.GetLibrary().GetCell("NAND2");                                           \
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("Y",   PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("B",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDir::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDir::INOUT,   PinKind::PWR);

#define CREATE_HALF_ADDER_CELL(ctx, name)                                                       \
        ctx.GetLibrary().AddCell("HALF_ADDER");                                                 \
        Cell &c1 = ctx.GetLibrary().GetCell("HALF_ADDER");                                      \
        c1.SetKind(CellKind::COMBINATIONAL);                                                    \
        c1.AddPin("CO",  PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("S",   PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("A",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("B",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("VDD", PinDir::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDir::INOUT,   PinKind::PWR);

#define CREATE_DFF_CKB_SB_RB_CELL(ctx, name)                                                    \
        ctx.GetLibrary().AddCell("DFF_CKB_SB_RB");                                              \
        Cell &c1 = ctx.GetLibrary().GetCell("DFF_CKB_SB_RB");                                   \
        c1.SetKind(CellKind::SEQUENTIAL);                                                       \
        c1.AddPin("Q",   PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("CKB", PinDir::IN,      PinKind::CLK);                                  \
        c1.AddPin("D",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("RB",  PinDir::IN,      PinKind::ASYNC);                                \
        c1.AddPin("SB",  PinDir::IN,      PinKind::ASYNC);                                \
        c1.AddPin("VDD", PinDir::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDir::INOUT,   PinKind::PWR);

#define CREATE_DFF_CK_SB_RB_CELL(ctx, name)                                                     \
        ctx.GetLibrary().AddCell("DFF_CK_SB_RB");                                               \
        Cell &c1 = ctx.GetLibrary().GetCell("DFF_CK_SB_RB");                                    \
        c1.SetKind(CellKind::SEQUENTIAL);                                                       \
        c1.AddPin("Q",   PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("CK",  PinDir::IN,      PinKind::CLK);                                  \
        c1.AddPin("D",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("RB",  PinDir::IN,      PinKind::ASYNC);                                \
        c1.AddPin("SB",  PinDir::IN,      PinKind::ASYNC);                                \
        c1.AddPin("VDD", PinDir::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDir::INOUT,   PinKind::PWR);

#define CREATE_LAT_CKB_R_S_CELL(ctx, name)                                                      \
        ctx.GetLibrary().AddCell("LAT_CKB_R_S");                                                \
        Cell &c1 = ctx.GetLibrary().GetCell("LAT_CKB_R_S");                                     \
        c1.SetKind(CellKind::SEQUENTIAL);                                                       \
        c1.AddPin("Q",   PinDir::OUT,     PinKind::DATA);                                 \
        c1.AddPin("D",   PinDir::IN,      PinKind::DATA);                                 \
        c1.AddPin("GB",  PinDir::IN,      PinKind::CLK);                                  \
        c1.AddPin("RB",  PinDir::IN,      PinKind::ASYNC);                                \
        c1.AddPin("SB",  PinDir::IN,      PinKind::ASYNC);                                \
        c1.AddPin("VDD", PinDir::INOUT,   PinKind::PWR);                                  \
        c1.AddPin("VSS", PinDir::INOUT,   PinKind::PWR);
