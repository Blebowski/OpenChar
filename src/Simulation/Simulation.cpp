

#include <vector>
#include <string>

#include "open_char.h"

#include "Utils.h"
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

void Simulation::SetSupply(Supply *supply)
{
    supply_ = supply;
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

    fprintf(f, ".title %s \n\n", name_);

    fprintf(f, "* Libraries\n");
    for (const auto &lib : libs_)
        fprintf(f, ".lib %s\n", lib);
    fprintf(f, "\n");

    fprintf(f, "* Include files\n");
    for (const auto &include : includes_)
        fprintf(f, ".include %s\n", include);
    fprintf(f, "\n");

    fprintf(f, ".temp %f\n", temp_);

    fprintf(f, "* Power Supply\n");
    fprintf(f, "Vpwr %s %s %f\n",  supply_->vdd_name_, supply_->gnd_name_, supply_->vdd_val_);
    fprintf(f, "VGnd %s GND %f\n\n", supply_->gnd_name_, supply_->gnd_val_);

    fprintf(f, "* Stimuli\n");
    for (const auto &s : stimuli_) {
        if (s.first == nullptr)
            continue;

        fprintf(f, "V%s %s %s ", s.first->name_, s.first->name_, supply_->gnd_name_);

        const Stimulus &v = s.second;
        if (v.kind_ == StimulusKind::CONSTANT)
            fprintf(f, "%f\n", v.volage_);
        else
            fprintf(f, "PULSE(%f %f %f %f %f %f)\n", v.v1_, v.v2_, v.t_delay_, v.t_rise_,
                    v.t_fall_, v.pulse_width_, v.period_, v.num_pulses_);
    }
    fprintf(f, "\n");

    fprintf(f, "* DUT\n", dut_title_);
    fprintf(f, "%s ", dut_title_);

    for (const auto &pin_p : dut_->GetPins(PinDirection::OUT))
        fprintf(f, "%s ", pin_p.name_);
    for (const auto &pin_p : dut_->GetPins(PinDirection::IN))
        fprintf(f, "%s ", pin_p.name_);
    for (const auto &pin_p : dut_->GetPins(PinKind::PWR))
        fprintf(f, "%s ", pin_p.name_);
    for (const auto &pin_p : dut_->GetPins(PinKind::GND))
        fprintf(f, "%s ", pin_p.name_);

    fprintf(f, "%s \n\n", dut_->name_);

    fprintf(f, ".control \n");
    if (kind_ == SimulationKind::TRAN)
        fprintf(f, "     tran %fFS %fFS\n", duration_, time_step_);
    else if (kind_ == SimulationKind::DC)
        fprintf(f, "     DC VGnd 0 0 0.1\n");

    std::filesystem::path wave = sim_dir / wave_file_;
    fprintf(f, "     write %s all\n", wave);

    fprintf(f, "     exit\n");
    fprintf(f, ".endc\n");
    fprintf(f, ".end\n");

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
    std::string cmd = sprintf("ngspice %s > %s", tb_path.c_str(), log_path.c_str());

    return system(cmd.c_str());
}

Waves Simulation::ReadWaves()
{
    std::filesystem::path sim_dir(name_);
    std::filesystem::path wave = sim_dir / wave_file_;

    return Waves(wave);
}

}