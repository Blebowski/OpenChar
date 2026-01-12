

#ifndef SIMULATION_H
#define SIMULATION_H

#include "open_char.h"
#include "Pin.h"
#include "Cell.h"
#include "Stimulus.h"
#include "Waves.h"

#include <vector>
#include <string>

namespace open_char {

class Simulation {

    public:
        Simulation(Cell *dut, double temp, double duration, double time_step);
        Simulation(Cell *dut, double temp);

        void AddInclude(std::string &include);
        void AddLib(std::string &lib);
        void SetVcc(std::string &name, double val);
        void SetVss(std::string &name, double val);
        void AddStimuli(Pin *pin, Stimulus &&stim);

        int Simulate();
        Waves* ReadWaves();

    private:
        SimulationKind kind_;
        Cell *dut_;
        double temp_;

        // Transient simulation details - always in femto-seconds
        double duration_;
        double time_step_;

        std::vector<std::string> includes_;
        std::vector<std::string> libs_;

        std::pair<std::string, double> vcc_;
        std::pair<std::string, double> vss_;

        std::vector<std::pair<Pin*, Stimulus>> stimuli_;

        std::string netlist_path_;

        const std::string simulation_name_ = "OPEN_CHAR_SIMULATION";
        const std::string dut_title_ = "XDUT";
        const std::string wave_file_ = "data.raw";

        void WriteNetlist();
};

};

#endif
