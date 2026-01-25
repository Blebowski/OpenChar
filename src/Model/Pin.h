
#ifndef PIN_H
#define PIN_H

#include <string>

#include "open_char.h"
#include "LogicTable.h"

namespace open_char {

class Pin {

    public:
        const std::string name_;

        const PinDirection direction_;
        const PinKind kind_;

        LogicTable logic_table_;

        Pin(std::string name, PinDirection direction, PinKind kind);
};

}

#endif