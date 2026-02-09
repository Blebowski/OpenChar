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

    enum class TemplateKind {
        DELAY,
        POWER
    };

    enum class WaveKind {
        TIME,
        VOLTAGE,
        CURRENT
    };

    typedef double Celsius;
    typedef double Volt;
    typedef double NanoSecond;
    typedef double MiliAmp;
    typedef double PicoFarad;
    typedef double KiloOhm;
    typedef double NanoWatt;

}

#endif