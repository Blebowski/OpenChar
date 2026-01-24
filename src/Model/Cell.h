
#ifndef CELL_H
#define CELL_H

#include <string>
#include <map>

#include "Pin.h"

namespace open_char {

class Cell {

    public:
        std::string name_;
        std::map<std::string, Pin> pins_;

        Cell(std::string name);
};

}

#endif