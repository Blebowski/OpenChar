
#ifndef LIBRARY_H
#define LIBRARY_H

#include "Cell.h"

namespace open_char {

class Library {

    public:
        Library();

        std::pair<Cell&, bool> AddCell(std::string name);
        bool HasCell(std::string name);
        Cell &GetCell(std::string name);

    private:
        std::map<std::string, Cell> cells_;

};

}

#endif