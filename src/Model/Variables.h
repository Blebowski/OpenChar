
#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>

#include "open_char.h"

namespace open_char {

class Variables {

    public:
        Variables();

        std::string GetRunDirectory();
        void SetRunDirectory(std::string &run_directory);

    private:
        std::string run_directory_;
};

}

#endif