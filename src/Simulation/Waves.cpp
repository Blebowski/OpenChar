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
            kind_ = WaveKind::TIME;
            reference_.reserve(n_points);
        } else if (strncmp(curr, "v(v-sweep)", 10) == 0) {
            kind_ = WaveKind::VSWEEP;
            reference_.reserve(n_points);
        } else {
            while (*curr != '(')
                curr++;

            char *start = curr;

            while (*curr != ')')
                curr++;

            key = std::string(start + 1 , curr - start - 1);
            for (auto & c: key)
                c = toupper(c);

            MOVE_TILL_SPACE(curr);
            MOVE_TILL_CHAR(curr);

            std::string curr_str = curr;
            curr_str.erase(std::remove(curr_str.begin(), curr_str.end(), '\n'), curr_str.end());
            if (curr_str == "voltage") {
                sig_names.push_back(key);
                voltages_[key] = std::pair<size_t, std::vector<double>>();
                voltages_[key].first = i;
                voltages_[key].second.reserve(n_points);
            } else if (curr_str == "current") {
                // Current always measured through voltage sources with "V" prefix and named equally as the node
                key.erase(0, 1);
                sig_names.push_back(key);
                currents_[key] = std::pair<size_t, std::vector<double>>();
                currents_[key].first = i;
                currents_[key].second.reserve(n_points);
            } else
                error("Invalid wave kind: '%s'\n", curr_str);
        }
    }

    // TODO: Check line read OK
    line = NULL;
    len = 0;
    getline(&line, &len, f);
    free(line);

    for (size_t sample = 0; sample < n_points; sample++) {

        // Assume first read is always reference -> True for NGSPICE TRAN and DC!
        double val;
        fread(&val, sizeof(double), 1, f);

        // Convert to nanoseconds
        if (kind_ == WaveKind::TIME)
            val *= 1E9;

        reference_.push_back(val);

        size_t i = 1;
        for (const auto &sig_name : sig_names) {
            double val;
            fread(&val, sizeof(double), 1, f);

            if (currents_.contains(sig_name)) {
                if (currents_[sig_name].first == i) {
                    // Convert to microamps
                    val *= 1E6;
                    currents_[sig_name].second.push_back(val);
                }
            }

            if (voltages_.contains(sig_name)) {
                if (voltages_[sig_name].first == i) {
                    voltages_[sig_name].second.push_back(val);
                }
            }

            i++;
        }
    }

    // Croscheck simulator dumps equal lenghts for all members
    size_t first_len = voltages_.cbegin()->second.second.size();
    for (const auto & sig : voltages_)
        assert(first_len == sig.second.second.size());
    for (const auto & sig : currents_)
        assert(first_len == sig.second.second.size());

    fclose(f);
}

void Waves::Print()
{
    std::map<std::string, size_t> lens;
    size_t total = 24; // TODO: Adjust first column to number of samples!

    total += voltages_.size() * 12;
    total += currents_.size() * 12;
    total += 2;

    PRINT_LINE(total);

    // TODO: Print various reference based on analysis type!
    printf("|      Index | Reference |");
    for (const auto &sig : voltages_) {
        printf(" %9s |", "V(" + sig.first + ")");
    }
    for (const auto &sig : currents_) {
        printf(" %9s |", "I(" + sig.first + ")");
    }
    printf(" \n");

    PRINT_LINE(total);

    for (size_t i = 0; i < voltages_.begin()->second.second.size(); i++) {
        printf("| %10d | %9f |", i, reference_[i]);
        for (const auto &sig : voltages_) {
            std::string fmt_str = sprintf(" %%%d.7f |", lens[sig.first]);
            printf(fmt_str, sig.second.second[i]);
        }
        for (const auto &sig : currents_) {
            std::string fmt_str = sprintf(" %%%d.7f |", lens[sig.first]);
            printf(fmt_str, sig.second.second[i]);
        }
        printf(" \n");
    }

    PRINT_LINE(total);
}

const std::vector<Volt>& Waves::GetVoltage(const std::string &node_name)
{
    assert (voltages_.contains(node_name));
    return voltages_[node_name].second;
}

const std::vector<MicroAmp>& Waves::GetCurrent(const std::string &node_name)
{
    assert (currents_.contains(node_name));
    return currents_[node_name].second;
}

NanoSecond Waves::GetTimeAtIndex(size_t index)
{
    assert (kind_ == WaveKind::TIME);
    return reference_[index];
}


size_t Waves::FindTransitionIndex(std::string name, int from, Volt th)
{
    assert (voltages_.contains(name));

    const std::vector<Volt>& d = voltages_[name].second;
    size_t len = d.size();

    // TODO: Cross-check first and last data match the "from" and "to".

    size_t index = len - 1;
    size_t step = len / 2;

    while (step > 0) {
        Volt v = d[index];

        if (v > th) {
            if (from == 0) {
                index -= step;
            } else {
                index += step;
            }
        } else {
            if (from == 0) {
                index += step;
            } else {
                index -= step;
            }
        }

        step /= 2;
    }

    if (index > d.size() - 1)
        index = d.size() - 1;

    return index;
}

NanoSecond Waves::FindTransitionTime(std::string name, int from, Volt th)
{
    return GetTimeAtIndex(FindTransitionIndex(name, from, th));
}

}