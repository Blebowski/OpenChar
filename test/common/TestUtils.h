
#include "StackTrace.h"
#include "Utils.h"

#define RUN_SIMULATIONS(ctx, prep_func)                                                             \
        do {                                                                                        \
            ctx.GetSimulationPool().Start();                                                        \
            prep_func;                                                                              \
            ctx.GetSimulationPool().WaitDone();                                                     \
        } while (0)

#define ALG_TEST_INIT(ctx, algs)                                                                    \
        StackTraceInit();                                                                           \
        Context ctx(nullptr);                                                                       \
        Algorithms algs(&ctx);
