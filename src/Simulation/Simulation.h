

#ifndef SIMULATION_H
#define SIMULATION_H

#include "open_char.h"
#include "Pin.h"
#include "Stimulus.h"

#include <vector>
#include <string>

namespace open_char {

class Simulation {

    public:
        std::vector<std::string> includes;
        std::vector<std::string> libs;

        double temp;

        std::pair<std::string, double> vcc;
        std::pair<std::string, double> vss;

        std::vector<std::pair<Pin*, Stimulus>> stimulus;

        Cell *dut;

        Simulation();
        void WriteNetlist();

    private:
        std::string netlist_path;
};

};

#endif
