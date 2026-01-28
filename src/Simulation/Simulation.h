

#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>
#include <filesystem>

#include "open_char.h"
#include "Pin.h"
#include "Cell.h"
#include "Stimulus.h"
#include "Waves.h"

namespace open_char {

class Simulation {

    public:
        Simulation(std::string name, Cell *dut, double duration, double time_step);
        Simulation(std::string name, Cell *dut);

        void SetVcc(std::pair<std::string, double> &v);
        void SetVss(std::pair<std::string, double> &v);
        void SetTemp(double temp);

        void AddInclude(const std::string include);
        void AddLib(std::string lib);
        void AddStimuli(Pin *pin, Stimulus &&stim);

        int Simulate();
        Waves ReadWaves();

    private:

        const std::string name_;
        const SimulationKind kind_;
        Cell * const dut_;

        double temp_;

        // Transient simulation details - always in femto-seconds
        double duration_;
        double time_step_;

        std::vector<std::string> includes_;
        std::vector<std::string> libs_;

        std::pair<std::string, double> vcc_;
        std::pair<std::string, double> vss_;

        std::vector<std::pair<Pin*, Stimulus>> stimuli_;

        const std::string testbench_ = "tb.cdl";
        const std::string dut_title_ = "XDUT";
        const std::string wave_file_ = "data.raw";
        const std::string log_file_ = "sim.log";

        void WriteTestBench();
};

};

#endif
