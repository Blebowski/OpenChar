
#ifndef OPCOND_H
#define OPCOND_H

#include <utility>

#include "open_char.h"
#include "Supply.h"

namespace open_char {

class OpCond {

    public:
        std::string name_;
        double temp_;
        Supply *supply_;

        OpCond(Supply *supply);
};

}

#endif