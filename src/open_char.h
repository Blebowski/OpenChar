#ifndef OPEN_CHAR
#define OPEN_CHAR

#include "Utils.h"

namespace open_char {

    // Data Model
    class Cell;
    class Context;
    class Expression;
    class Library;
    class OpCond;
    class Pin;
    class Supply;
    class Template;
    class TimingArgc;
    class Variables;

    // Simulation backends
    class Algorithms;
    class Stimulus;
    class Simulation;
    class SimulationPool;
    class Waves;

    // TCL Commands
    class TclCmd;
    class TclCmdOpt;

    // Exposed routines;
    void CreateTclCommands(Context *ctx);
    void RegisterTclCommands(Context *ctx);
    void CreateTclVariables(Context *ctx);

    #define PIN_DIR(x)          \
        x(IN)                   \
        x(OUT)                  \
        x(INOUT)
    OPENCHAR_ENUM(PinDir, PIN_DIR)
    #undef PIN_DIR

    #define PIN_KIND(x)         \
        x(PWR)                  \
        x(GND)                  \
        x(DATA)                 \
        x(CLK)                  \
        x(ASYNC)
    OPENCHAR_ENUM(PinKind, PIN_KIND)
    #undef PIN_KIND

    #define CELL_KIND(x)        \
        x(COMBINATIONAL)        \
        x(SEQUENTIAL)
    OPENCHAR_ENUM(CellKind, CELL_KIND)
    #undef CELL_KIND

    #define SEQ_KIND(x)         \
        x(FLIP_FLOP)            \
        x(LATCH)
    OPENCHAR_ENUM(SeqKind, SEQ_KIND)
    #undef SEQ_KIND

    #define STIM_KIND(x)        \
        x(PULSE)                \
        x(CONSTANT)             \
        x(PWL)
    OPENCHAR_ENUM(StimKind, STIM_KIND)
    #undef STIM_KIND

    #define SIM_KIND(x)         \
        x(TRAN)                 \
        x(DC)
    OPENCHAR_ENUM(SimKind, SIM_KIND)
    #undef SIM_KIND

    #define SIM_CLASS(x)        \
        x(SANITY)               \
        x(ICAP)                 \
        x(LEAKAGE)              \
        x(COM_LOGTBL)           \
        x(COM_DLYTRANPWR)       \
        x(SEQ_ASYNCFUNC)        \
        x(SEQ_EDGEORLVL)        \
        x(FF_CKPOL)             \
        x(FF_DLYTRANPWR)        \
        x(FF_SETUP)             \
        x(FF_HOLD)              \
        x(FF_CK_MPW)
    OPENCHAR_ENUM(SimClass, SIM_CLASS)
    #undef SIM_CLASS

    #define WAVE_KIND(x)        \
        x(TIME)                 \
        x(VSWEEP)
    OPENCHAR_ENUM(WaveKind, WAVE_KIND)
    #undef WAVE_KIND

    #define TEMPL_KIND(x)       \
        x(DELAY)                \
        x(POWER)                \
        x(CONSTRAINT)
    OPENCHAR_ENUM(TemplKind, TEMPL_KIND)
    #undef TEMPL_KIND

    #define EDGE_KIND(x)        \
        x(RISING)               \
        x(FALLING)
    OPENCHAR_ENUM(EdgeKind, EDGE_KIND)
    #undef EDGE_KIND

    #define EXPR_KIND(x)  \
        x(AND)                  \
        x(OR)                   \
        x(XOR)                  \
        x(NOT)                  \
        x(TERM)                 \
        x(CONSTANT)
    OPENCHAR_ENUM(ExprKind, EXPR_KIND)
    #undef EXPR_KIND

    #define EXPR_EQUAL_KIND(x)  \
        x(EQUAL)                \
        x(INVERT)
    OPENCHAR_ENUM(ExprEqualKind, EXPR_EQUAL_KIND)
    #undef EXPR_EQUAL_KIND

    #define ASYNC_PRIO(x)       \
        x(PRESET)               \
        x(CLEAR)
    OPENCHAR_ENUM(AsyncPrio, ASYNC_PRIO)
    #undef ASYNC_PRIO

    #define ARC_KIND(x)         \
        x(COMBO)                \
        x(SEQ_CK)               \
        x(SEQ_SET)              \
        x(SEQ_CLR)              \
        x(SEQ_SETUP)            \
        x(SEQ_HOLD)             \
        x(SEQ_MPW)
    OPENCHAR_ENUM(ArcKind, ARC_KIND)
    #undef ARC_KIND

    #define UNATE_KIND(x)       \
        x(POSITIVE_UNATE)       \
        x(NEGATIVE_UNATE)       \
        x(NON_UNATE)
    OPENCHAR_ENUM(UnateKind, UNATE_KIND)
    #undef UNATE_KIND

    #define CHARACT_STATE(x)        \
        x(START)                    \
        x(SANITY)                   \
        x(INPUT_CAP)                \
        x(LEAKAGE)                  \
        x(COM_LOG_TBL)              \
        x(COM_DLY_PWR)              \
        x(SEQ_ASYNC_FUNCS)          \
        x(SEQ_CELL_KIND)            \
        x(SEQ_FF_CK_POL)            \
        x(SEQ_FF_DLY_PWR)           \
        x(SEQ_FF_SETUP_START)       \
        x(SEQ_FF_SETUP_FINISH)      \
        x(SEQ_FF_HOLD_START)        \
        x(SEQ_FF_HOLD_FINISH)       \
        x(SEQ_FF_CK_MPW_START)      \
        x(SEQ_FF_CK_MPW_FINISH)     \
        x(ERROR)                    \
        x(DONE)
    OPENCHAR_ENUM(CharactState, CHARACT_STATE)
    #undef CHARACT_STATE


    typedef double Celsius;
    typedef double Volt;
    typedef double NanoSecond;
    typedef double MicroAmp;
    typedef double PicoFarad;
    typedef double KiloOhm;
    typedef double NanoWatt;
    typedef double PicoJoule;

}

#endif
