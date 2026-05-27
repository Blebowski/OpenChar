
#include "StackTrace.h"

#define RUN_SIMULATIONS(ctx, prep_func)                                                             \
        do {                                                                                        \
            ctx.GetSimulationPool().Start();                                                        \
            prep_func;                                                                              \
            ctx.GetSimulationPool().WaitDone();                                                     \
        } while (0)