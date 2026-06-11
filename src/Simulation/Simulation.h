

#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include <mutex>

#include "open_char.h"
#include "Pin.h"
#include "Cell.h"
#include "Stimulus.h"
#include "Waves.h"
#include "Supply.h"

namespace open_char {

class Simulation {

    public:
        Simulation(Context *ctx, std::string name, Cell *dut, SimKind kind);

        void SetSupply(Supply *supply);
        void SetTemp(Celsius temp);

        void AddInclude(const std::string include);
        void AddModel(std::string model);
        void AddStimuli(Pin *pin, Stimulus &&stim);
        void AddLoad(Pin *pin, PicoFarad cap);

        // TODO: Somehow unify the metadata handling for arbitrary types
        void PutMetaData(int data);
        int GetMetaDataAt(size_t index);

        void PutDoubleMetaData(double data);
        double GetDoubleMetaDataAt(size_t index);

        int Simulate();
        Waves ReadWaves();

        void SetTimeStep(NanoSecond time_step);
        NanoSecond GetTimeStep();

        void SetDuration(NanoSecond duration);

        bool IsFinished();
        bool CheckSucesfull();

        const std::string name_;

    private:

        const SimKind kind_;
        Cell * const dut_;

        Celsius temp_;

        NanoSecond duration_;

        // TODO: Make precision configurable by user!
        NanoSecond time_step_ = 0.001;

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

        std::vector<int> metadata_;
        std::vector<double> double_metadata_;

        int exit_code_ = 0;
        bool is_finished_;
        std::mutex lock_;

        // TODO: Make series shunts on inputs configurable
        size_t r_counter = 0;
        KiloOhm in_shunt = 1E-12; //  1 microOhm should be sufficiently low

        void WriteTestBench();
};

};

#endif
