
#include <fmt/printf.h>

#include "open_char.h"
#include "Waves.h"

namespace open_char {

#define MOVE_TILL_CHAR(ptr) while (*ptr == ' ' || *ptr == '\t') ptr++
#define MOVE_TILL_SPACE(ptr) while (*ptr != ' ' && *ptr != '\t') ptr++
#define PRINT_LINE(len) fmt::printf("%s\n", std::string (len, '-'));

Waves::Waves(std::string path)
{
    FILE *f = fopen(path.c_str(), "r");
    if (f == NULL) {
        // TODO: Replace with some logging
        fmt::printf("Failed to open file: %s\n", path);
        return;
    }

    char *line;
    size_t len;
    size_t n_vars;
    size_t n_points;

    std::vector<std::string> sig_names;

    // TODO: Read date  (second line - index 1) if we need it
    // TODO: Read flags (fourth line - index 4) and figure if other than "real" can exist!

    for (size_t i = 0; i < 7; i++) {

        // TODO: Check line read OK
        line = NULL;
        len = 0;
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
            break;
        default:
            break;
        }
    }

    for (size_t i = 0; i < n_vars; i++) {

        line = NULL;
        len = 0;

        // TODO: Check line read OK
        getline(&line, &len, f);

        char *curr = line;

        MOVE_TILL_CHAR(curr);
        MOVE_TILL_SPACE(curr);
        MOVE_TILL_CHAR(curr);

        std::string key;
        if (strncmp(curr, "time ", 5) == 5) {
            key = std::string(curr);
        } else {
            while (*curr != '(')
                curr++;

            char *start = curr;

            while (*curr != ')')
                curr++;

            key = std::string(start + 1 , curr - start - 1);
            for (auto & c: key)
                c = toupper(c);
        }

        data_[key] = std::vector<double>();
        data_[key].reserve(n_points);
        sig_names.push_back(key);

        MOVE_TILL_SPACE(curr);
        MOVE_TILL_CHAR(curr);

        data_kind_[key] = std::string(curr);
    }

    // TODO: Check line read OK
    line = NULL;
    len = 0;
    getline(&line, &len, f);
    free(line);

    for (size_t sample = 0; sample < n_points; sample++) {
        for (const auto &sig_name : sig_names) {
            double val;
            // TODO: Check size of double was read properly!
            fread(&val, sizeof(double), 1, f);
            data_[sig_name].push_back(val);
        }
    }

    fclose(f);
}

void Waves::Print()
{
    std::map<std::string, size_t> lens;
    size_t total = 0;

    for (const auto &sig : data_) {
        size_t col_width = sig.first.size();

        if (col_width < 8)
            col_width = 8;

        lens[sig.first] = col_width;
        total += col_width + 3;
    }
    total += 1;

    PRINT_LINE(total);

    fmt::printf("|");
    for (const auto &sig : data_) {
        std::string fmt_str = fmt::sprintf(" %%%ds |", lens[sig.first]);
        fmt::printf(fmt_str, sig.first);
    }
    fmt::printf(" \n");

    PRINT_LINE(total);

    for (size_t i = 0; i < data_.begin()->second.size(); i++) {
        fmt::printf("|");
        for (const auto &sig : data_) {
            std::string fmt_str = fmt::sprintf(" %%%d.4f |", lens[sig.first]);
            fmt::printf(fmt_str, sig.second[i]);
        }
        fmt::printf(" \n");
    }

    PRINT_LINE(total);
}

}