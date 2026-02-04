
#ifndef SUPPLY_H
#define SUPPLY_H

#include <utility>
#include <string>

#include "open_char.h"

namespace open_char {

class Supply {

    public:
        std::string vdd_name_;
        double vdd_val_;

        std::string gnd_name_;
        double gnd_val_;

        Supply(std::string vdd_name, double vdd_val, std::string gnd_name, double gnd_val);
};

}

#endif