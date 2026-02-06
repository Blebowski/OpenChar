#ifndef OPEN_CHAR
#define OPEN_CHAR

#include "tcl.h"

namespace open_char {

    // Data Model
    class Cell;
    class Context;
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

}

#endif