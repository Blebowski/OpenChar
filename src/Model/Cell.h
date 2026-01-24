
#ifndef CELL_H
#define CELL_H

#include <string>
#include <vector>

#include "Pin.h"

namespace open_char {

class Cell {

    public:
        const std::string name_;
        std::vector<Pin> pins_;

        Cell(const char *name);
};

}

#endif