
#ifndef SUPPLY_H
#define SUPPLY_H

#include <utility>
#include <string>

#include "open_char.h"

namespace open_char {

class Supply {

    public:
        std::string vdd_name_;
        Volt vdd_val_;

        std::string gnd_name_;
        Volt gnd_val_;

        Supply(std::string vdd_name, Volt vdd_val, std::string gnd_name, Volt gnd_val);
};

}

#endif