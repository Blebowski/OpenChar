#ifndef OPEN_CHAR
#define OPEN_CHAR

namespace open_char {

    // Data Model
    class LogicTable;
    class Pin;
    class Cell;

    // Simulation backends
    class Stimulus;
    class Simulation;

    enum class PinDirection {
        IN,
        OUT,
        INOUT
    };

    enum class PinKind {
        PWR,
        GROUND,
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