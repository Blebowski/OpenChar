
#include <fmt/printf.h>

#include "open_char.h"
#include "Waves.h"

namespace open_char {

#define MOVE_TILL_CHAR(ptr) while (*ptr == ' ') ptr++
#define MOVE_TILL_SPACE(ptr) while (*ptr != ' ') ptr++

Waves::Waves(std::string path)
{
    // TODO: Check file exists !
    FILE *f = fopen(path.c_str(), "r");

    char *line;
    size_t len;
    size_t n_vars;
    size_t n_points;

    std::vector<std::string> sig_names;

    // TODO: Read date  (second line - index 1) if we need it
    // TODO: Read flags (fourth line - index 4) and figure if other than "real" can exist!

    for (size_t i = 0; i < 7; i++) {
        // TODO: Check line read OK
        getline(&line, &len, f);

        switch (i) {
        case 0:
            title_ = std::string(&(line[7]));
            break;
        case 2:
            plot_name_ = std::string(&(line[10]));
            break;
        case 4:
            n_vars = atoi(&(line[15]));
            break;
        case 5:
            n_points = atoi(&(line[12]));
        }
    }

    for (size_t i = 0; i < n_vars; i++) {

        // TODO: Check line read OK
        getline(&line, &len, f);

        MOVE_TILL_CHAR(line);
        MOVE_TILL_SPACE(line);
        MOVE_TILL_CHAR(line);

        std::string key;
        if (strncmp(line, "time", 4))
            key = std::string("time");
        else {
            while (*line != '(') line++;
            char *start = line;
            while (*line != ')') line++;
            key = std::string(start, line - start);
        }

        data_[key] = std::vector<double>();
        data_[key].reserve(n_points);
        sig_names.push_back(key);

        MOVE_TILL_SPACE(line);
        MOVE_TILL_CHAR(line);

        data_kind_[key] = std::string(line);
    }

    // TODO: Check line read OK
    getline(&line, &len, f);

    for (size_t sample = 0; sample < n_points; sample++) {
        for (const auto &sig_name : sig_names) {
            double val;
            // TODO: Check size of double was read properly!
            fread(&val, sizeof(double), 1, f);
            data_[sig_name].push_back(val);
        }
    }
}

}