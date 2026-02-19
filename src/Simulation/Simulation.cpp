

#include <vector>
#include <string>

#include "open_char.h"

#include "Context.h"
#include "Utils.h"
#include "Simulation.h"

namespace open_char {

Simulation::Simulation(Context *ctx, std::string name, Cell *dut, SimulationKind kind) :
    name_(name),
    kind_(kind),
    dut_(dut),
    duration_(10)
{
    // TODO: Handle exceptions where creating Run directory fails!
    std::filesystem::path run_dir(ctx->GetVariables().GetVariable("run_directory"));
    std::filesystem::create_directory(run_dir);

    std::filesystem::path cell_dir = run_dir / dut->GetName();
    std::filesystem::create_directory(cell_dir);

    sim_dir_ = cell_dir / name;
    std::filesystem::create_directory(sim_dir_);
}

void Simulation::AddInclude(std::string include)
{
    includes_.push_back(include);
}

void Simulation::AddModel(std::string model)
{
    models_.push_back(model);
}

void Simulation::SetSupply(Supply *supply)
{
    supply_ = supply;
}

void Simulation::SetTemp(Celsius temp)
{
    temp_ = temp;
}

void Simulation::AddStimuli(Pin *pin, Stimulus &&stim)
{
    stimuli_.push_back(std::pair<Pin*, Stimulus>(pin, stim));
}

void Simulation::AddLoad(Pin *pin, PicoFarad cap)
{
    loads_.push_back(std::pair<Pin *,PicoFarad>(pin, cap));
}

void Simulation::WriteTestBench()
{
    std::string tb_path =  sim_dir_ / testbench_;
    FILE *f = fopen(tb_path.c_str(), "w");

    fprintf(f, ".title %s \n\n", name_);

    fprintf(f, "* Libraries / Models \n");
    for (const auto &model : models_)
        fprintf(f, ".lib %s\n", model);
    fprintf(f, "\n");

    fprintf(f, "* Include files\n");
    for (const auto &include : includes_)
        fprintf(f, ".include %s\n", include);
    fprintf(f, "\n");

    fprintf(f, ".temp %f\n", temp_);

    fprintf(f, "* Power Supply\n");
    fprintf(f, "V%s %s %s %f\n", supply_->GetVddName(), supply_->GetVddName(),
                                 supply_->GetGndName(), supply_->GetVddVoltage());
    fprintf(f, "V%s %s GND %f\n\n", supply_->GetGndName(), supply_->GetGndName(),
                                    supply_->GetGndVoltage());

    fprintf(f, "* Stimuli\n");
    for (const auto &s : stimuli_) {
        if (s.first == nullptr)
            continue;

        fprintf(f, "V%s %s %s ", s.first->name_, s.first->name_, supply_->GetGndName());

        const Stimulus &v = s.second;
        if (v.kind_ == StimulusKind::CONSTANT)
            fprintf(f, "%f\n", v.volage_);
        else
            fprintf(f, "PULSE(%fV %fV %fNS %fNS %fNS %fNS)\n", v.v1_, v.v2_, v.t_delay_, v.t_rise_,
                    v.t_fall_, v.pulse_width_, v.period_, v.num_pulses_);
    }
    fprintf(f, "\n");

    fprintf(f, "* Loads\n");
    for (const auto &l : loads_) {
        fprintf(f, "V%s %s %s_VSRC 0\n", l.first->name_, l.first->name_, l.first->name_);
        fprintf(f, "C%s %s_VSRC %s %fpF\n", l.first->name_, l.first->name_,
                                       supply_->GetGndName(), l.second);
    }
    fprintf(f, "\n");

    fprintf(f, "* DUT\n", dut_title_);
    fprintf(f, "%s ", dut_title_);

    for (const auto &pin : dut_->GetPins(PinDirection::OUT))
        fprintf(f, "%s ", pin.name_);
    for (const auto &pin : dut_->GetPins(PinDirection::IN))
        fprintf(f, "%s ", pin.name_);
    for (const auto &pin : dut_->GetPins(PinKind::PWR))
        fprintf(f, "%s ", pin.name_);
    for (const auto &pin : dut_->GetPins(PinKind::GND))
        fprintf(f, "%s ", pin.name_);

    fprintf(f, "%s \n\n", dut_->GetName());

    fprintf(f, "* WAVEFORM DUMP\n");
    for (const auto &pin : dut_->GetPins()) {
        fprintf(f, ".SAVE %s\n", pin.name_);
    }
    fprintf(f, ".SAVE i(V%s)\n", supply_->GetVddName());
    for (const auto &l : loads_) {
        fprintf(f, ".SAVE i(V%s)\n", l.first->name_);
    }

    fprintf(f, "\n");

    if (kind_ == SimulationKind::TRAN)
        fprintf(f, ".TRAN %s %fNS\n", duration_, time_step_);
    else if (kind_ == SimulationKind::DC)
        fprintf(f, ".DC V%s 0 0 0.1\n", supply_->GetGndName());

    fprintf(f, ".end\n");

    fclose(f);
}

int Simulation::Simulate()
{
    WriteTestBench();

    std::filesystem::path tb_path = sim_dir_ / testbench_;
    std::filesystem::path log_path = sim_dir_ / log_file_;
    std::filesystem::path wave_path = sim_dir_ / wave_file_;
    std::filesystem::path stdout_path = sim_dir_ / std_out_file_;

    std::string cmd = sprintf("ngspice %s -b -r %s -o %s > %s 2>&1", tb_path.c_str(),
                                wave_path.c_str(), log_path.c_str(), stdout_path.c_str());

    int exit_code = system(cmd.c_str());
    return exit_code;
}

Waves Simulation::ReadWaves()
{
    std::filesystem::path wave_path = sim_dir_ / wave_file_;

    return Waves(wave_path);
}

}