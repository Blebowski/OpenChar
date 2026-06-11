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
    void RegisterTclCommands(Context *ctx);
    void CreateTclVariables(Context *ctx);

    #define PIN_DIRECTION(x)    \
        x(IN)                   \
        x(OUT)                  \
        x(INOUT)
    OPENCHAR_ENUM(PinDirection, PIN_DIRECTION)
    #undef PIN_DIRECTION

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

    #define SEQUENTIAL_KIND(x)  \
        x(FLIP_FLOP)            \
        x(LATCH)
    OPENCHAR_ENUM(SequentialKind, SEQUENTIAL_KIND)
    #undef SEQUENTIAL_KIND

    #define STIMULUS_KIND(x)    \
        x(PULSE)                \
        x(CONSTANT)             \
        x(PWL)
    OPENCHAR_ENUM(StimulusKind, STIMULUS_KIND)
    #undef STIMULUS_KIND

    #define SIMULATION_KIND(x)  \
        x(TRAN)                 \
        x(DC)
    OPENCHAR_ENUM(SimulationKind, SIMULATION_KIND)
    #undef SIMULATION_KIND

    #define SIMULATION_CLASS(x) \
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
    OPENCHAR_ENUM(SimulationClass, SIMULATION_CLASS)
    #undef SIMULATION_CLASS

    #define WAVE_KIND(x)        \
        x(TIME)                 \
        x(VSWEEP)
    OPENCHAR_ENUM(WaveKind, WAVE_KIND)
    #undef WAVE_KIND

    #define TEMPLATE_KIND(x)    \
        x(DELAY)                \
        x(POWER)                \
        x(CONSTRAINT)
    OPENCHAR_ENUM(TemplateKind, TEMPLATE_KIND)
    #undef TEMPLATE_KIND

    #define EDGE_KIND(x)        \
        x(RISING)               \
        x(FALLING)
    OPENCHAR_ENUM(EdgeKind, EDGE_KIND)
    #undef EDGE_KIND

    #define EXPRESSION_KIND(x)  \
        x(AND)                  \
        x(OR)                   \
        x(XOR)                  \
        x(NOT)                  \
        x(TERM)                 \
        x(CONSTANT)
    OPENCHAR_ENUM(ExpressionKind, EXPRESSION_KIND)
    #undef EXPRESSION_KIND

    #define EXPRESSION_EQUALITY_KIND(x) \
        x(EQUAL)                        \
        x(INVERT)
    OPENCHAR_ENUM(ExpressionEqualityKind, EXPRESSION_EQUALITY_KIND)
    #undef EXPRESSION_EQUALITY_KIND

    #define ASYNC_PRIORITY(x)   \
        x(PRESET)               \
        x(CLEAR)
    OPENCHAR_ENUM(AsyncPriority, ASYNC_PRIORITY)
    #undef ASYNC_PRIORITY

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
