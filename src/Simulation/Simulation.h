

#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>
#include <filesystem>
#include <functional>

#include "open_char.h"
#include "Pin.h"
#include "Cell.h"
#include "Stimulus.h"
#include "Waves.h"
#include "Supply.h"

namespace open_char {

class Simulation {

    public:
        Simulation(Context *ctx, std::string name, Cell *dut, SimulationKind kind);

        void SetSupply(Supply *supply);
        void SetTemp(Celsius temp);

        void AddInclude(const std::string include);
        void AddModel(std::string model);
        void AddStimuli(Pin *pin, Stimulus &&stim);
        void AddLoad(Pin *pin, PicoFarad cap);

        int Simulate();
        Waves ReadWaves();

        NanoSecond GetTimeStep();

        void SetPostSimCb(std::function<int(void)> post_sim_cb);
        void ExecutePostSimCb();

    private:

        const std::string name_;
        const SimulationKind kind_;
        Cell * const dut_;

        Celsius temp_;

        NanoSecond duration_;

        // TODO: Precision may be configurable either manually by user,
        //       or somehow determine based on quickness of response.
        NanoSecond time_step_ = 0.0001;

        std::vector<std::string> includes_;
        std::vector<std::string> models_;

        Supply *supply_;

        std::vector<std::pair<Pin*, Stimulus>> stimuli_;
        std::vector<std::pair<Pin*, PicoFarad>> loads_;

        const std::string testbench_ = "tb.cdl";
        const std::string dut_title_ = "XDUT";
        const std::string wave_file_ = "data.raw";
        const std::string log_file_ = "sim.log";
        const std::string std_out_file_ = "stdout.log";

        std::filesystem::path sim_dir_;
        std::function<int(void)> post_sim_cb_;

        void WriteTestBench();
};

};

#endif
