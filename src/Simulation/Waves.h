
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

        const std::vector<double>& GetData(const std::string &name);
        size_t GetDataLen();
        double GetDataAtIndex(const std::string &name, size_t index);
        WaveKind GetKind(const std::string &name);

    private:
        std::map<std::string, std::pair<std::vector<double>, WaveKind>> data_;
};

}

#endif