
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

        std::string title_;
        std::string plot_name_;

        std::map<std::string, std::vector<double>> data_;
        std::map<std::string, std::string> data_kind_;
};

}

#endif