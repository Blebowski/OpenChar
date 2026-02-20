
#ifndef WAVES_H
#define WAVES_H

#include <map>
#include <string>
#include <vector>
#include <ctime>

#include "open_char.h"

namespace open_char {

class Waves {

    public:
        Waves(std::string path);
        void Print();

        std::string title_;
        std::string plot_name_;

        const std::vector<Volt>& GetVoltage(const std::string &node_name);
        const std::vector<MicroAmp>& GetCurrent(const std::string &node_name);

        NanoSecond FindTimeOfVoltageMonotonic(std::string name, int from, Volt th);

        NanoSecond GetTimeAtIndex(size_t index);

    private:
        std::map<std::string, std::pair<size_t, std::vector<Volt>>> voltages_;
        std::map<std::string, std::pair<size_t, std::vector<MicroAmp>>> currents_;
        std::vector<double> reference_;
        WaveKind kind_;
};

}

#endif