

#include <vector>
#include <string>

#include <fmt/printf.h>

#include "open_char.h"
#include "Simulation.h"

namespace open_char {

Simulation::Simulation(std::string name, Cell *dut, double duration, double time_step) :
    name_(name),
    kind_(SimulationKind::TRAN),
    dut_(dut),
    duration_(duration),
    time_step_(time_step)
{}

Simulation::Simulation(std::string name, Cell *dut) :
    name_(name),
    kind_(SimulationKind::DC),
    dut_(dut),
    duration_(0),
    time_step_(0)
{}

void Simulation::AddInclude(std::string include)
{
    includes_.push_back(include);
}

void Simulation::AddLib(std::string lib)
{
    libs_.push_back(lib);
}

void Simulation::SetVcc(std::pair<std::string, double> &v)
{
    vcc_ = v;
}

void Simulation::SetVss(std::pair<std::string, double> &v)
{
    vss_ = v;
}

void Simulation::SetTemp(double temp)
{
    temp_ = temp;
}

void Simulation::AddStimuli(Pin *pin, Stimulus &&stim)
{
    stimuli_.push_back(std::pair<Pin*, Stimulus>(pin, stim));
}

void Simulation::WriteTestBench()
{
    std::filesystem::path sim_dir(name_);
    std::string tb_path =  sim_dir / testbench_;
    FILE *f = fopen(tb_path.c_str(), "w");

    fmt::print(f, ".title {}\n\n", name_);

    fmt::print(f, "* Libraries\n");
    for (const auto &lib : libs_)
        fmt::print(f, ".lib {}\n", lib);
    fmt::print(f, "\n");

    fmt::print(f, "* Include files\n");
    for (const auto &include : includes_)
        fmt::print(f, ".include {}\n", include);
    fmt::print(f, "\n");

    fmt::print(f, ".temp {}\n", temp_);

    fmt::print(f, "* Power Supply\n");
    fmt::print(f, "Vpwr {} {} {}\n", vcc_.first, vss_.first, vcc_.second);
    fmt::print(f, "VGnd {} GND {}\n\n", vss_.first, vss_.second);

    fmt::print(f, "* Stimuli\n");
    for (const auto &s : stimuli_) {
        if (s.first == nullptr)
            continue;

        std::string_view pn = s.first->name_;
        const Stimulus &v = s.second;
        fmt::print(f, "V{} {} {} ", pn, pn, vss_.first);

        if (v.kind_ == StimulusKind::CONSTANT)
            fmt::print(f, "{}\n", v.volage_);
        else
            fmt::print(f, "PULSE({} {} {} {} {} {})\n", v.v1_, v.v2_, v.t_delay_, v.t_rise_,
                       v.t_fall_, v.pulse_width_, v.period_, v.num_pulses_);
    }
    fmt::print(f, "\n");

    fmt::print(f, "* DUT\n", dut_title_);
    fmt::print(f, "{} ", dut_title_);

    for (const auto &pin_p : dut_->GetPins(PinDirection::OUT))
        fmt::print(f, "{} ", pin_p.name_);
    for (const auto &pin_p : dut_->GetPins(PinDirection::IN))
        fmt::print(f, "{} ", pin_p.name_);
    for (const auto &pin_p : dut_->GetPins(PinKind::PWR))
        fmt::print(f, "{} ", pin_p.name_);
    for (const auto &pin_p : dut_->GetPins(PinKind::GND))
        fmt::print(f, "{} ", pin_p.name_);

    fmt::print(f, "{} \n\n", dut_->name_);

    fmt::print(f, ".control \n");
    if (kind_ == SimulationKind::TRAN)
        fmt::print(f, "     tran {}FS {}FS\n", duration_, time_step_);
    else if (kind_ == SimulationKind::DC)
        fmt::print(f, "     DC VGnd 0 0 0.1\n");

    std::filesystem::path wave = sim_dir / wave_file_;
    fmt::print(f, "     write {} all\n", wave.c_str());

    fmt::print(f, "     exit\n");
    fmt::print(f, ".endc\n");
    fmt::print(f, ".end\n");

    fclose(f);
}

int Simulation::Simulate()
{
    // TODO: Check exists and check created OK!
    std::filesystem::path sim_dir(name_);
    std::filesystem::create_directory(sim_dir);

    WriteTestBench();

    std::filesystem::path tb_path = sim_dir / testbench_;
    std::filesystem::path log_path = sim_dir / log_file_;

    // TODO: Redirect also Error output!
    std::string cmd = fmt::sprintf("ngspice %s > %s", tb_path.c_str(), log_path.c_str());

    return system(cmd.c_str());
}

Waves* Simulation::ReadWaves()
{
    return new Waves(wave_file_);
}

}