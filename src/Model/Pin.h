
#ifndef PIN_H
#define PIN_H

#include <string>

#include "open_char.h"
#include "LogicTable.h"

namespace open_char {

class Pin {

    public:
        std::string name_;

        PinDirection direction_;
        PinKind kind_;

        LogicTable logic_table_;

        Pin();
};

}

#endif