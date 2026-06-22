#include <cmath>
#include <algorithm>

#include "open_char.h"

#include "Waves.h"
#include "Utils.h"

namespace open_char {

#define MOVE_TILL_CHAR(ptr) while (*ptr == ' ' || *ptr == '\t') ptr++
#define MOVE_TILL_SPACE(ptr) while (*ptr != ' ' && *ptr != '\t') ptr++
#define PRINT_LINE(len) printf("%s\n", std::string(len, '-'));

#define READ_LINE_AND_CHECK(line, len, f)                                           \
            do {                                                                    \
                line = NULL;                                                        \
                len = 0;                                                            \
                ssize_t read = getline(&line, &len, f);                             \
                if (read == -1) {                                                   \
                    fatal("Failed to read line from waveform file.");               \
                }                                                                   \
            } while (0)

#define RAED_DOUBLE_AND_CHECK(val, f)                                               \
            do {                                                                    \
                size_t n = fread(&val, sizeof(double), 1, f);                       \
                if (n != 1) {                                                       \
                    fatal("Failed to read floating number from waveform file.");    \
                }                                                                   \
            } while (0)

Waves::Waves(std::string path)
{
    FILE *f = fopen(path.c_str(), "r");
    if (f == NULL) {
        fatal("Failed to open file: %s\n", path);
        return;
    }

    char *line;
    size_t len;
    size_t n_vars = 0;
    size_t n_points = 0;

    std::vector<std::string> sig_names;

    // TODO: Read date  (second line - index 1) if we need it
    // TODO: Read flags (fourth line - index 4) and figure if other than "real" can exist!

    for (size_t i = 0; i < 7; i++) {
        READ_LINE_AND_CHECK(line, len, f);

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
        free(line);
    }

    for (size_t i = 0; i < n_vars; i++) {

        READ_LINE_AND_CHECK(line, len, f);
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
            for (size_t i = 0; i < key.size(); i++)
                key[i] = static_cast<char>(toupper(key[i]));

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

        free(line);
    }

    READ_LINE_AND_CHECK(line, len, f);
    free(line);

    for (size_t sample = 0; sample < n_points; sample++) {

        // Assume first read is always reference -> True for NGSPICE TRAN and DC!
        double val;
        RAED_DOUBLE_AND_CHECK(val, f);

        // Convert to nanoseconds
        if (kind_ == WaveKind::TIME)
            val *= 1E9;

        reference_.push_back(val);

        size_t i = 1;
        for (const auto &sig_name : sig_names) {

            RAED_DOUBLE_AND_CHECK(val, f);

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

size_t Waves::GetIndexOfTime(NanoSecond time)
{
    assert (kind_ == WaveKind::TIME);

    for (size_t i = 0; i < reference_.size(); i++) {
        if (reference_[i] > time)
            return i;
    }

    assert(false);
    return 0;
}

size_t Waves::FindReferenceIndex(double val)
{
    double step = static_cast<double>(reference_.size()) / 2;
    double index = step;

    while (step > 1) {
        step /= 2;

        double low_t = reference_[static_cast<size_t>(index - step)];
        double high_t = reference_[static_cast<size_t>(index + step)];

        if (std::abs(low_t - val) < std::abs(high_t - val)) {
            index -= step;
        } else {
            index += step;
        }
    }

    return static_cast<size_t>(index);
}

size_t Waves::FindTransitionIndex(std::string name, Volt th,
                                  NanoSecond time_start, NanoSecond time_end)
{
    assert(voltages_.contains(name));
    assert(kind_ == WaveKind::TIME);
    assert(time_start < time_end);

    size_t index_start = FindReferenceIndex(time_start);
    size_t index_stop = FindReferenceIndex(time_end);

    assert(index_stop < reference_.size() && index_start <= index_stop);

    const std::vector<Volt>& d = voltages_[name].second;

    size_t len = index_stop - index_start;
    double step = static_cast<double>(len) / 2;
    double index = static_cast<double>(index_start) + step;

    assert(index_start < d.size());
    assert(index_stop  < d.size());

    Volt v_first = d[index_start];

    while (step > 1.0) {
        step /= 2;

        if (d[static_cast<size_t>(index)] < th) {
            if (th < v_first) {
                index -= step;
            } else {
                index += step;
            }
        } else {
            if (th < v_first) {
                index += step;
            } else {
                index -= step;
            }
        }
    }

    assert(static_cast<size_t>(index) < d.size());

    return static_cast<size_t>(index);
}

NanoSecond Waves::FindTransitionTime(std::string name, Volt th,
                                     NanoSecond time_start, NanoSecond time_end)
{
    return GetTimeAtIndex(FindTransitionIndex(name, th, time_start, time_end));
}

size_t Waves::FindTransitionIndex(std::string name, Volt th)
{
    assert (voltages_.contains(name));
    assert(kind_ == WaveKind::TIME);

    return FindTransitionIndex(name, th, reference_.front() , reference_.back());
}

NanoSecond Waves::FindTransitionTime(std::string name, Volt th)
{
    return GetTimeAtIndex(FindTransitionIndex(name, th));
}

}