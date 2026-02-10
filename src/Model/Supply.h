
#ifndef SUPPLY_H
#define SUPPLY_H

#include <utility>
#include <string>

#include "open_char.h"

namespace open_char {

class Supply {

    public:
        Supply(std::string vdd_name, Volt vdd_val, std::string gnd_name, Volt gnd_val);

        void SetGndName(std::string &name);
        void SetVddName(std::string &name);
        void SetGndVoltage(Volt value);
        void SetVddVoltage(Volt value);

        std::string GetGndName();
        Volt GetGndVoltage();
        std::string GetVddName();
        Volt GetVddVoltage();

    private:
        std::string vdd_name_;
        std::string gnd_name_;

        Volt vdd_val_;
        Volt gnd_val_;
};

}

#endif