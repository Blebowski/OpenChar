
#ifndef CELL_H
#define CELL_H

#include <string>
#include <map>

#include "Pin.h"

namespace open_char {

class Cell {

    public:
        const std::string name_;
        std::map<std::string, Pin> pins_;

        Cell(std::string name);
        std::pair<Pin&, bool> AddPin(std::string name, PinDirection direction, PinKind kind);
};

}

#endif