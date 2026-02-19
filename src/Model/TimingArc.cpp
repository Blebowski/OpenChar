

#include "Cell.h"
#include "TimingArc.h"
#include "Pin.h"
#include "Template.h"
#include "Utils.h"

namespace open_char {

TimingArc::TimingArc(Pin *pin, Template *templ, int64_t in_a,
                     int64_t in_b, int out_a, int out_b) :
    pin_(pin),
    template_(templ),
    in_a_(in_a),
    in_b_(in_b),
    out_a_(out_a),
    out_b_(out_b)
{}

void TimingArc::AddRiseDelay(size_t row, NanoSecond delay)
{
    while (rise_delays_.size() <= row)
        rise_delays_.push_back(std::vector<NanoSecond>());
    rise_delays_[row].push_back(delay);
}

void TimingArc::AddFallDelay(size_t row, NanoSecond delay)
{
    while (fall_delays_.size() <= row)
        fall_delays_.push_back(std::vector<NanoSecond>());
    fall_delays_[row].push_back(delay);
}

std::vector<std::vector<NanoSecond>>& TimingArc::GetRiseDelays()
{
    return rise_delays_;
}

std::vector<std::vector<NanoSecond>>& TimingArc::GetFallDelays()
{
    return fall_delays_;
}

void TimingArc::AddRiseTransition(size_t row, NanoSecond transition)
{
    while (rise_transitions_.size() <= row)
        rise_transitions_.push_back(std::vector<NanoSecond>());
    rise_transitions_[row].push_back(transition);
}

void TimingArc::AddFallTransition(size_t row, NanoSecond transition)
{
    while (fall_transitions_.size() <= row)
        fall_transitions_.push_back(std::vector<NanoSecond>());
    fall_transitions_[row].push_back(transition);
}

std::vector<std::vector<NanoSecond>>& TimingArc::GetRiseTransitions()
{
    return rise_transitions_;
}

std::vector<std::vector<NanoSecond>>& TimingArc::GetFallTransitions()
{
    return fall_transitions_;
}

void TimingArc::Print()
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

Pin* TimingArc::GetRelatedPin()
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

bool TimingArc::isPositiveUnate()
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

void TimingArc::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "timing () {\n");
    tab++;

    Pin* rel_pin = GetRelatedPin();
    TAB_FPRINTF(tab, f, "related_pin : %s ;\n", rel_pin->name_);
    TAB_FPRINTF(tab, f, "timing_type : combinational ;\n");
    TAB_FPRINTF(tab, f, "timing_sense : %s_unate ;\n",
                        (isPositiveUnate()) ? "positive" : "negative");

    if (rise_delays_.size() > 0) {
        TAB_FPRINTF(tab, f, "cell_rise() {\n");
        tab++;

        template_->WriteLiberty(tab, f);

        TAB_FPRINTF(tab, f, "values ( \\\n");
        tab++;

        for (const auto & row : rise_delays_) {
            TAB_FPRINTF(tab, f, "\"");
            size_t i = 0;
            for (const NanoSecond & d : row) {
                fprintf(f, "%f%s", d, (i < row.size() - 1) ? ", " : "");
                i++;
            }
            fprintf(f, "\" \\\n");
        }

        tab--;
        TAB_FPRINTF(tab, f, ") ;\n");

        tab--;
        TAB_FPRINTF(tab, f, "} /* end cell_rise */\n");
    }

    if (rise_transitions_.size() > 0) {
        TAB_FPRINTF(tab, f, "rise_transition() {\n");
        tab++;

        template_->WriteLiberty(tab, f);

        TAB_FPRINTF(tab, f, "values ( \\\n");
        tab++;

        for (const auto & row : rise_transitions_) {
            TAB_FPRINTF(tab, f, "\"");
            size_t i = 0;
            for (const NanoSecond & d : row) {
                fprintf(f, "%f%s", d, (i < row.size() - 1) ? ", " : "");
                i++;
            }
            fprintf(f, "\" \\\n");
        }

        tab--;
        TAB_FPRINTF(tab, f, ") ;\n");

        tab--;
        TAB_FPRINTF(tab, f, "} /* end rise_transition */\n");
    }

    if (fall_delays_.size() > 0) {
        TAB_FPRINTF(tab, f, "cell_fall() {\n");
        tab++;

        template_->WriteLiberty(tab, f);

        TAB_FPRINTF(tab, f, "values ( \\\n");
        tab++;

        for (const auto & row : fall_delays_) {
            TAB_FPRINTF(tab, f, "\"");
            size_t i = 0;
            for (const NanoSecond & d : row) {
                fprintf(f, "%f%s", d, (i < row.size() - 1) ? ", " : "");
                i++;
            }
            fprintf(f, "\" \\\n");
        }

        tab--;
        TAB_FPRINTF(tab, f, ") ;\n");

        tab--;
        TAB_FPRINTF(tab, f, "} /* end cell_rise */\n");
    }

    if (fall_transitions_.size() > 0) {
        TAB_FPRINTF(tab, f, "fall_transition() {\n");
        tab++;

        template_->WriteLiberty(tab, f);

        TAB_FPRINTF(tab, f, "values ( \\\n");
        tab++;

        for (const auto & row : fall_transitions_) {
            TAB_FPRINTF(tab, f, "\"");
            size_t i = 0;
            for (const NanoSecond & d : row) {
                fprintf(f, "%f%s", d, (i < row.size() - 1) ? ", " : "");
                i++;
            }
            fprintf(f, "\" \\\n");
        }

        tab--;
        TAB_FPRINTF(tab, f, ") ;\n");

        tab--;
        TAB_FPRINTF(tab, f, "} /* end rise_transitions */\n");
    }
    tab--;

    TAB_FPRINTF(tab, f, "} /* end timing */\n");
}

}
