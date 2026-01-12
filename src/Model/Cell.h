
#ifndef CELL_H
#define CELL_H

#include <string>
#include <vector>

#include "Pin.h"

namespace open_char {

class Cell {

    std::string name;

    std::vector<Pin> pins;

    Cell();
};

}

#endif