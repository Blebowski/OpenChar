
#ifndef OPCOND_H
#define OPCOND_H

#include <utility>

#include "open_char.h"
#include "Supply.h"

namespace open_char {

class OpCond {

    public:
        OpCond(Supply *supply);

        void SetName(std::string name);
        const std::string& GetName();

        void SetTemperature(Celsius temperature);
        Celsius GetTemperature();

        Supply* GetSupply();
        void SetSupply(Supply *supply);

        void WriteLiberty(FILE *f, size_t tab);

    private:
        std::string name_;
        Supply *supply_;
        Celsius temperature_;
};

}

#endif