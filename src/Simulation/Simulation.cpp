////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenChar - VLSI library characterizer
// Copyright (C) 2026  Ondrej Ille
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <cmath>

#include "open_char.h"

#include "Context.h"
#include "Pin.h"
#include "Cell.h"
#include "Utils.h"
#include "Simulation.h"

namespace open_char {

Simulation::Simulation(Context *ctx, SimClass sim_class, std::string name, Cell *dut, SimKind kind) :
    name_(name),
    class_(sim_class),
    kind_(kind),
    dut_(dut),
    duration_(10),
    is_finished_(false)
{
    // TODO: Handle exceptions where creating Run directory fails!
    std::filesystem::path run_dir(ctx->GetVariables().GetVariable("run_directory"));
    std::filesystem::create_directory(run_dir);

    std::filesystem::path cell_dir = run_dir / dut->GetName();
    std::filesystem::create_directory(cell_dir);

    sim_dir_ = cell_dir / (toString(class_) + "_" + name);
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
    fprintf(f, ".options method=gear\n\n");

    fprintf(f, "* Power Supply\n");
    fprintf(f, "V%s %s %s %f\n", supply_->GetVddName(), supply_->GetVddName(),
                                 supply_->GetGndName(), supply_->GetVddVoltage());
    fprintf(f, "V%s %s GND %f\n\n", supply_->GetGndName(), supply_->GetGndName(),
                                    supply_->GetGndVoltage());

    fprintf(f, "* Stimuli\n");
    for (const auto &s : stimuli_) {
        if (s.first == nullptr)
            continue;

        fprintf(f, "V%s %s_I %s ", s.first->name_, s.first->name_, supply_->GetGndName());

        const Stimulus &v = s.second;
        if (v.kind_ == StimKind::CONSTANT) {
            fprintf(f, "%f\n", v.volage_);
        } else if (v.kind_ == StimKind::PULSE) {
            fprintf(f, "PULSE(%fV %fV %fNS %fNS %fNS %fNS %fNS %d)\n", v.v1_, v.v2_, v.t_delay_, v.t_rise_,
                       v.t_fall_, v.pulse_width_, v.period_, v.num_pulses_);
        } else if (v.kind_ == StimKind::PWL) {
            fprintf(f, "PWL(");
            for (const auto & [voltage, time] : s.second.pwl_vals_) {
                fprintf(f, "%fNS %fV ", time, voltage);
            }
            fprintf(f, ")\n");
        }

        fprintf(f, "R%d %s_I %s %fK \n\n", r_counter, s.first->name_, s.first->name_, in_shunt);
        r_counter++;
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

    for (const auto &pin : dut_->GetPins(PinDir::OUT))
        fprintf(f, "%s ", pin.name_);
    for (const auto &pin : dut_->GetPins(PinDir::IN))
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
    for (const auto &s : stimuli_) {
        fprintf(f, ".SAVE i(V%s)\n", s.first->name_);
    }

    fprintf(f, "\n");

    if (kind_ == SimKind::TRAN) {
        fprintf(f, ".TRAN %dFS %dNS \n",
                    static_cast<int>(std::round(time_step_ * 1E6)),
                    static_cast<int>(std::round(duration_)));

    } else if (kind_ == SimKind::DC) {
        fprintf(f, ".DC V%s 0 0 0.1\n", supply_->GetGndName());
    }

    fprintf(f, ".end\n");

    fclose(f);
}

int Simulation::Simulate()
{
    {
        std::unique_lock<std::mutex> local_lock(lock_);
        assert(is_finished_ == false);
    }

    WriteTestBench();

    std::filesystem::path tb_path = sim_dir_ / testbench_;
    std::filesystem::path log_path = sim_dir_ / log_file_;
    std::filesystem::path wave_path = sim_dir_ / wave_file_;
    std::filesystem::path stdout_path = sim_dir_ / std_out_file_;

    std::string cmd = sprintf("ngspice %s -b -r %s -o %s > %s 2>&1", tb_path.c_str(),
                                wave_path.c_str(), log_path.c_str(), stdout_path.c_str());

    exit_code_ = system(cmd.c_str());
    {
        std::unique_lock<std::mutex> local_lock(lock_);
        is_finished_ = true;
    }

    return exit_code_;
}

Waves Simulation::ReadWaves()
{
    {
        std::unique_lock<std::mutex> local_lock(lock_);
        assert(is_finished_);
    }
    std::filesystem::path wave_path = sim_dir_ / wave_file_;

    return Waves(wave_path);
}

void Simulation::SetTimeStep(NanoSecond time_step)
{
    time_step_ = time_step;
}

NanoSecond Simulation::GetTimeStep()
{
    return time_step_;
}

void Simulation::SetDuration(NanoSecond duration)
{
    duration_ = duration;
}

void Simulation::PutMetaData(int data)
{
    metadata_.push_back(data);
}

int Simulation::GetMetaDataAt(size_t index)
{
    assert(index < metadata_.size());
    return metadata_[index];
}

void Simulation::PutDoubleMetaData(double data)
{
    double_metadata_.push_back(data);
}

double Simulation::GetDoubleMetaDataAt(size_t index)
{
    assert(index < double_metadata_.size());
    return double_metadata_[index];
}

bool Simulation::IsFinished()
{
    bool rv;
    {
        std::unique_lock<std::mutex> local_lock(lock_);
        rv = is_finished_;
    }
    return rv;
}

bool Simulation::CheckSucesfull()
{
    assert (is_finished_);
    if (exit_code_ != 0) {
        error("%s - %s simulation failed with exit code %d", dut_->GetName(), name_, exit_code_);

        std::filesystem::path log_path = sim_dir_ / log_file_;
        error("%s - %s simulation log:", dut_->GetName(), name_);

        FILE *log_f = fopen(log_path.c_str(), "r");
        if (log_f == NULL) {
            fatal("Failed to open log file: %s\n", log_path);
            return false;
        }

        size_t i = 1;
        while (true) {
            char *line = NULL;
            size_t len = 0;
            // TODO: Check getline result
            ssize_t rv = getline(&line, &len, log_f);
            if (rv == -1) {
                free(line);
                break;
            }
            printf("%d:   %s", i, line);
            free(line);
            i++;
        }
        fclose(log_f);

        return false;
    }
    return true;
}

}