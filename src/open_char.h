#ifndef OPEN_CHAR
#define OPEN_CHAR

#include "tcl.h"

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

    enum class PinDirection {
        IN,
        OUT,
        INOUT
    };

    enum class PinKind {
        PWR,
        GND,
        DATA,
        CLK,
        ASYNC
    };

    enum class CellKind {
        COMBINATIONAL,
        SEQUENTIAL
    };

    enum class SequentialKind {
        FLIP_FLOP,
        LATCH
    };

    enum class StimulusKind {
        PULSE,
        CONSTANT
    };

    enum class SimulationKind {
        TRAN,
        DC
    };

    enum class WaveKind {
        TIME,
        VSWEEP
    };

    enum class TemplateKind {
        DELAY,
        POWER,
        CONSTRAINT
    };

    enum class EdgeKind {
        RISING,
        FALLING
    };

    enum class ExpressionKind {
        AND,
        OR,
        XOR,
        NOT,
        TERM,
        CONSTANT
    };

    enum class ExpressionEqualityKind {
        EQUAL,
        INVERT
    };

    enum class AsyncPriority {
        PRESET,
        CLEAR
    };

    enum class ArcKind {
        COMBO,
        SEQ_CK,
        SEQ_SET,
        SEQ_CLR,
        SEQ_SETUP,
        SEQ_HOLD
    };

    enum class UnateKind {
        POSITIVE_UNATE,
        NEGATIVE_UNATE,
        NON_UNATE
    };

    enum class CharactState {
        START,
        SANITY,
        INPUT_CAP,

        COM_LOG_TBL_LKG,
        COM_DLY_PWR,

        SEQ_ASYNC_FUNCS,
        SEQ_CELL_KIND,
        SEQ_FF_CK_POL,
        SEQ_FF_DLY_PWR,
        SEQ_FF_SETUP_START,
        SEQ_FF_SETUP_FINISH,
        SEQ_FF_HOLD_START,
        SEQ_FF_HOLD_FINISH,

        ERROR,
        DONE,
    };

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