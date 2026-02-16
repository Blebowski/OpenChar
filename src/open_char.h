#ifndef OPEN_CHAR
#define OPEN_CHAR

#include "tcl.h"

namespace open_char {

    // Data Model
    class Cell;
    class Context;
    class DelayTable;
    class Library;
    class LogicTable;
    class Pin;
    class Supply;
    class Template;

    // Simulation backends
    class Algorithms;
    class Stimulus;
    class Simulation;
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
        POWER
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

    typedef double Celsius;
    typedef double Volt;
    typedef double NanoSecond;
    typedef double MicroAmp;
    typedef double PicoFarad;
    typedef double KiloOhm;
    typedef double NanoWatt;

}

#endif