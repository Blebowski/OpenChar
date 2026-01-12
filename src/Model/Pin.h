
#ifndef PIN_H
#define PIN_H

#include <string>

#include "open_char.h"
#include "LogicTable.h"

namespace open_char {

class Pin {

    std::string name;

    PinDirection direction;
    PinKind kind;

    LogicTable *logic_table;

    Pin();
};

}

#endif