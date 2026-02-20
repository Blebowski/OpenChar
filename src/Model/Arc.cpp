

#include "Cell.h"
#include "Arc.h"
#include "Pin.h"
#include "Template.h"
#include "Utils.h"

namespace open_char {

Arc::Arc(Pin *pin, Template *templ, int64_t in_a, int64_t in_b, int out_a, int out_b) :
    pin_(pin),
    template_(templ),
    in_a_(in_a),
    in_b_(in_b),
    out_a_(out_a),
    out_b_(out_b)
{}

void Arc::AddRiseDelay(size_t row, NanoSecond delay)
{
    while (rise_delays_.size() <= row)
        rise_delays_.push_back(std::vector<NanoSecond>());
    rise_delays_[row].push_back(delay);
}

void Arc::AddFallDelay(size_t row, NanoSecond delay)
{
    while (fall_delays_.size() <= row)
        fall_delays_.push_back(std::vector<NanoSecond>());
    fall_delays_[row].push_back(delay);
}

std::vector<std::vector<NanoSecond>>& Arc::GetRiseDelays()
{
    return rise_delays_;
}

std::vector<std::vector<NanoSecond>>& Arc::GetFallDelays()
{
    return fall_delays_;
}

void Arc::AddRiseTransition(size_t row, NanoSecond transition)
{
    while (rise_transitions_.size() <= row)
        rise_transitions_.push_back(std::vector<NanoSecond>());
    rise_transitions_[row].push_back(transition);
}

void Arc::AddFallTransition(size_t row, NanoSecond transition)
{
    while (fall_transitions_.size() <= row)
        fall_transitions_.push_back(std::vector<NanoSecond>());
    fall_transitions_[row].push_back(transition);
}

std::vector<std::vector<NanoSecond>>& Arc::GetRiseTransitions()
{
    return rise_transitions_;
}

std::vector<std::vector<NanoSecond>>& Arc::GetFallTransitions()
{
    return fall_transitions_;
}

void Arc::AddRisePower(size_t row, PicoJoule energy)
{
    while (rise_power_.size() <= row)
        rise_power_.push_back(std::vector<PicoJoule>());
    rise_power_[row].push_back(energy);
}

void Arc::AddFallPower(size_t row, PicoJoule energy)
{
    while (fall_power_.size() <= row)
        fall_power_.push_back(std::vector<PicoJoule>());
    fall_power_[row].push_back(energy);
}

std::vector<std::vector<PicoJoule>>& Arc::GetRisePowers()
{
    return rise_power_;
}

std::vector<std::vector<NanoSecond>>& Arc::GetFallPowers()
{
    return fall_power_;
}

void Arc::Print()
{
    size_t width = std::max(rise_delays_[0].size() * 10 + 1,
                            fall_delays_[0].size() * 10 + 1);

    PRINT_LINE(width)

    printf("Inputs A: %08b\n", in_a_);
    printf("Inputs B: %08b\n", in_b_);
    printf("Output A: %8b\n",  out_a_);
    printf("Output B: %8b\n",  out_b_);

    PRINT_LINE(width)

    printf("Output Rise delays:\n");

    PRINT_LINE(width)

    for (const auto &row : rise_delays_) {
        printf("|");
        for (const auto & cell: row) {
            printf(" %7.5f |", cell);
        }
        printf("\n");
    }

    PRINT_LINE(width)

    printf("Output Fall delays:\n");

    PRINT_LINE(width)

    for (const auto &row : fall_delays_) {
        printf("|");
        for (const auto & cell: row) {
            printf(" %7.5f |", cell);
        }
        printf("\n");
    }

    PRINT_LINE(width)
}

Pin* Arc::GetRelatedPin()
{
    int64_t diff = in_a_ ^ in_b_;

    // Must have exactly one bit set -> One pin transitioning
    assert (IS_POWER_OF_2(diff));

    int pos = __builtin_ctzll(diff);
    Pin *rel_pin = nullptr;

    int i = 0;
    for (auto &p : pin_->cell_->GetPins(PinDirection::IN)) {
        if (i == pos) {
            rel_pin = &(p);
        }
        i++;
    }
    assert(rel_pin != nullptr);

    return rel_pin;
}

bool Arc::isPositiveUnate()
{
    int64_t diff = in_a_ ^ in_b_;

    // Must have exactly one bit set -> One pin transitioning
    assert (IS_POWER_OF_2(diff));

    int pos = __builtin_ctzll(diff);

    int bit_a = (in_a_ >> pos) & 0x1;
    int bit_b = (in_b_ >> pos) & 0x1;

    assert ((bit_a == 0 && bit_b == 1) || (bit_a == 1 && bit_b == 0));
    assert ((out_a_ == 0 && out_b_ == 1) || (out_b_ == 0 && out_a_ == 1));

    if (bit_a == 0 && bit_b == 1) {
        if (out_a_ == 0 && out_b_ == 1) {
            return true;
        } else {
            return false;
        }
    } else {
        if (out_a_ == 0 && out_b_ == 1) {
            return false;
        } else {
            return true;
        }
    }
}

void Arc::WriteTable(FILE *f, size_t tab, std::vector<std::vector<double>>& data,
                     std::string title)
{
    TAB_FPRINTF(tab, f, "%s() {\n", title);
    tab++;

    template_->WriteLiberty(tab, f);

    TAB_FPRINTF(tab, f, "values ( \\\n");
    tab++;

    for (const auto & row : data) {
        TAB_FPRINTF(tab, f, "\"");
        size_t i = 0;
        for (const double & d : row) {
            fprintf(f, "%f%s", d, (i < row.size() - 1) ? ", " : "");
            i++;
        }
        fprintf(f, "\" \\\n");
    }

    tab--;
    TAB_FPRINTF(tab, f, ") ;\n");

    tab--;
    TAB_FPRINTF(tab, f, "} /* end %s */\n", title);
}

void Arc::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "timing () {\n");
    tab++;

    Pin* rel_pin = GetRelatedPin();
    TAB_FPRINTF(tab, f, "related_pin : %s ;\n", rel_pin->name_);
    TAB_FPRINTF(tab, f, "timing_type : combinational ;\n");
    TAB_FPRINTF(tab, f, "timing_sense : %s_unate ;\n",
                        (isPositiveUnate()) ? "positive" : "negative");

    if (rise_delays_.size() > 0) {
        WriteTable(f, tab, rise_delays_, "cell_rise");
    }

    if (rise_transitions_.size() > 0) {
        WriteTable(f, tab, rise_transitions_, "rise_transition");
    }

    if (fall_delays_.size() > 0) {
        WriteTable(f, tab, fall_delays_, "cell_fall");
    }

    if (fall_transitions_.size() > 0) {
        WriteTable(f, tab, fall_transitions_, "fall_transition");
    }
    tab--;

    TAB_FPRINTF(tab, f, "} /* end timing */\n");

    TAB_FPRINTF(tab, f, "internal_power () {\n");
    tab++;

    TAB_FPRINTF(tab, f, "related_pin : %s ;\n", rel_pin->name_);

    if (rise_power_.size() > 0) {
        WriteTable(f, tab, rise_power_, "rise_power");
    }

    if (rise_power_.size() > 0) {
        WriteTable(f, tab, fall_power_, "fall_power");
    }

    tab--;
    TAB_FPRINTF(tab, f, "}\n");

}

}
