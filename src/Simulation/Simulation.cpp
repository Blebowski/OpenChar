

#include <vector>
#include <string>
#include <stdio.h>

#include <fmt/printf.h>

#include "open_char.h"
#include "Simulation.h"

namespace open_char {

Simulation::Simulation(Cell *dut, double temp, double duration, double time_step) :
    kind_(SimulationKind::TRAN),
    dut_(dut),
    temp_(temp),
    duration_(duration),
    time_step_(time_step)
{}

Simulation::Simulation(Cell *dut, double temp) :
    kind_(SimulationKind::DC),
    dut_(dut),
    temp_(temp),
    duration_(0),
    time_step_(0)
{}

void Simulation::AddInclude(std::string &include)
{
    includes_.push_back(include);
}

void Simulation::AddLib(std::string &lib)
{
    libs_.push_back(lib);
}

void Simulation::SetVcc(std::string &name, double val)
{
    vcc_ = std::pair<std::string, double> (name, val);
}

void Simulation::SetVss(std::string &name, double val)
{
    vss_ = std::pair<std::string, double> (name, val);
}

void Simulation::AddStimuli(Pin *pin, Stimulus &&stim)
{
    stimuli_.push_back(std::pair<Pin*, Stimulus>(pin, stim));
}

void Simulation::WriteNetlist()
{
    FILE *f = fopen(netlist_path_.c_str(), "w");

    fmt::print(f, ".title {}\n", simulation_name_);

    fmt::print(f, "* Libraries\n");
    for (const auto &lib : libs_)
        fmt::print(f, "{}\n", lib);
    fmt::print(f, "\n");

    fmt::print(f, "* Include files\n");
    for (const auto &include : includes_)
        fmt::print(f, "{}\n", include);
    fmt::print(f, "\n");

    fmt::print(f, ".temp {}\n", temp_);

    fmt::print(f, "* Power Supply\n");
    fmt::print(f, "Vpwr {} {} {}\n", vcc_.first, vss_.first, vcc_.second);
    fmt::print(f, "VGnd {} GND {}\n", vss_.first, vss_.second);

    for (const auto &s : stimuli_) {
        if (s.first == nullptr)
            continue;

        std::string_view pn = s.first->name_;
        const Stimulus &v = s.second;
        fmt::print(f, "V{} {} {}", pn, pn, vss_.first);

        if (v.kind_ == StimulusKind::CONSTANT)
            fmt::print(f, "{}\n", v.volage_);
        else
            fmt::print(f, "PULSE({} {} {} {} {} {})\n", v.v1_, v.v2_, v.t_delay_, v.t_rise_,
                       v.t_fall_, v.pulse_width_, v.period_, v.num_pulses_);
    }

    fmt::print(f, "{} ", dut_title_);
    for (const auto &pin_p : dut_->pins_)
        fmt::print(f, "{} ", pin_p.second.name_);
    fmt::print(f, "{} \n\n", dut_->name_);

    fmt::print(f, ".control \n");
    if (kind_ == SimulationKind::TRAN)
        fmt::print(f, "     tran {}FS {}FS\n", duration_, time_step_);
    else if (kind_ == SimulationKind::DC)
        fmt::print(f, "     DC VGnd 0 0 0.1\n");
    fmt::print(f, "     write {} all\n", wave_file_);
    fmt::print(f, "     exit\n");
    fmt::print(f, ".endc\n");
}

int Simulation::Simulate()
{
    std::string cmd = fmt::sprintf("ngspice %s", netlist_path_.c_str());
    return system(cmd.c_str());
}

Waves* Simulation::ReadWaves()
{
    return new Waves(wave_file_);
}

}