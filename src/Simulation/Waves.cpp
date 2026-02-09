#include "open_char.h"

#include "Waves.h"
#include "Utils.h"

namespace open_char {

#define MOVE_TILL_CHAR(ptr) while (*ptr == ' ' || *ptr == '\t') ptr++
#define MOVE_TILL_SPACE(ptr) while (*ptr != ' ' && *ptr != '\t') ptr++
#define PRINT_LINE(len) printf("%s\n", std::string(len, '-'));

Waves::Waves(std::string path)
{
    FILE *f = fopen(path.c_str(), "r");
    if (f == NULL) {
        error("Failed to open file: %s\n", path);
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
        if (strncmp(curr, "time", 4) == 0) {
            key = "time";
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

        data_[key] = std::pair<std::vector<double>, WaveKind>();
        data_[key].first.reserve(n_points);
        sig_names.push_back(key);

        MOVE_TILL_SPACE(curr);
        MOVE_TILL_CHAR(curr);

        std::string curr_str = curr;
        curr_str.erase(std::remove(curr_str.begin(), curr_str.end(), '\n'), curr_str.end());
        if (curr_str == "time")
            data_[key].second = WaveKind::TIME;
        else if (curr_str == "voltage")
            data_[key].second = WaveKind::VOLTAGE;
        else if (curr_str == "current")
            data_[key].second = WaveKind::CURRENT;
        else
            error("Invalid wave kind: '%s'\n", curr_str);
    }

    // TODO: Check line read OK
    line = NULL;
    len = 0;
    getline(&line, &len, f);
    free(line);

    for (size_t sample = 0; sample < n_points; sample++) {
        for (const auto &sig_name : sig_names) {
            double val;
            fread(&val, sizeof(double), 1, f);

            if (sig_name == "time")
                val *= 1E9;

            // TODO: Convert current values to MiliAmps
            data_[sig_name].first.push_back(val);
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

    printf("|");
    for (const auto &sig : data_) {
        std::string fmt_str = sprintf(" %%%ds |", lens[sig.first]);
        printf(fmt_str, sig.first);
    }
    printf(" \n");

    PRINT_LINE(total);

    for (size_t i = 0; i < data_.begin()->second.first.size(); i++) {
        printf("|");
        for (const auto &sig : data_) {
            std::string fmt_str = sprintf(" %%%d.4f |", lens[sig.first]);
            printf(fmt_str, sig.second.first[i]);
        }
        printf(" \n");
    }

    PRINT_LINE(total);
}

const std::vector<double>& Waves::GetData(const std::string &name)
{
    return data_[name].first;
}

double Waves::GetDataAtIndex(const std::string &name, size_t index)
{
    return data_[name].first[index];
}

WaveKind Waves::GetKind(const std::string &name)
{
    return data_[name].second;
}

size_t Waves::GetDataLen()
{
    return data_.cbegin()->second.first.size();
}

}