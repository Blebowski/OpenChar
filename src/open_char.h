#ifndef OPEN_CHAR
#define OPEN_CHAR

#include "tcl.h"

namespace open_char {

    // Data Model
    class LogicTable;
    class Pin;
    class Cell;

    // Simulation backends
    class Algorithms;
    class Stimulus;
    class Simulation;

    // TCL Commands
    class TclCmd;
    class TclCmdOpt;

    // OpenChar run context
    class Context;

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


}

#endif