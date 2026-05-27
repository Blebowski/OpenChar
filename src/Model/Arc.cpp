

#include "Cell.h"
#include "Arc.h"
#include "Pin.h"
#include "Template.h"
#include "Utils.h"

namespace open_char {

Arc::Arc(Pin *pin, Template *templ, ArcKind kind, int64_t in_a,
         int64_t in_b, int out_a, int out_b) :
    pin_(pin),
    template_(templ),
    in_a_(in_a),
    in_b_(in_b),
    out_a_(out_a),
    out_b_(out_b),
    kind_(kind)
{}

#define ENLARGE_MATRIX(matrix, type, row_size, col_size, def_val)                                   \
            do {                                                                                    \
                while (matrix.size() < row_size) {                                                  \
                    matrix.push_back(std::vector<type>(col_size));                                  \
                }                                                                                   \
                for (auto & row : matrix) {                                                         \
                    while (row.size() < col_size) {                                                 \
                        row.push_back(def_val);                                                     \
                    }                                                                               \
                }                                                                                   \
            } while (0);                                                                            \

void Arc::SetRiseDelay(size_t row, size_t col, NanoSecond delay)
{
    ENLARGE_MATRIX(rise_delays_, NanoSecond, row + 1, col + 1, 0.0);
    rise_delays_[row][col] = delay;
}

void Arc::SetFallDelay(size_t row, size_t col, NanoSecond delay)
{
    ENLARGE_MATRIX(fall_delays_, NanoSecond, row + 1, col + 1, 0.0);
    fall_delays_[row][col] = delay;
}

std::vector<std::vector<NanoSecond>>& Arc::GetRiseDelays()
{
    return rise_delays_;
}

std::vector<std::vector<NanoSecond>>& Arc::GetFallDelays()
{
    return fall_delays_;
}

void Arc::SetRiseTransition(size_t row, size_t col, NanoSecond transition)
{
    ENLARGE_MATRIX(rise_transitions_, NanoSecond, row + 1, col + 1, 0.0);
    rise_transitions_[row][col] = transition;
}

void Arc::SetFallTransition(size_t row, size_t col, NanoSecond transition)
{
    ENLARGE_MATRIX(fall_transitions_, NanoSecond, row + 1, col + 1, 0.0);
    fall_transitions_[row][col] = transition;
}

std::vector<std::vector<NanoSecond>>& Arc::GetRiseTransitions()
{
    return rise_transitions_;
}

std::vector<std::vector<NanoSecond>>& Arc::GetFallTransitions()
{
    return fall_transitions_;
}

void Arc::SetRisePower(size_t row, size_t col, PicoJoule energy)
{
    ENLARGE_MATRIX(rise_powers_, PicoJoule, row + 1, col + 1, 0.0);
    rise_powers_[row][col] = energy;
}

void Arc::SetFallPower(size_t row, size_t col, PicoJoule energy)
{
    ENLARGE_MATRIX(fall_powers_, PicoJoule, row + 1, col + 1, 0.0);
    fall_powers_[row][col] = energy;
}

std::vector<std::vector<PicoJoule>>& Arc::GetRisePowers()
{
    return rise_powers_;
}

std::vector<std::vector<NanoSecond>>& Arc::GetFallPowers()
{
    return fall_powers_;
}

void Arc::SetRiseConstraint(size_t row, size_t col, NanoSecond constr)
{
    ENLARGE_MATRIX(rise_constraints_, NanoSecond, row + 1, col + 1, 0.0);
    rise_constraints_[row][col] = constr;
}

std::vector<std::vector<NanoSecond>>& Arc::GetRiseConstraints()
{
    return rise_constraints_;
}

void Arc::SetFallConstraint(size_t row, size_t col, NanoSecond constr)
{
    ENLARGE_MATRIX(fall_constraints_, NanoSecond, row + 1, col + 1, 0.0);
    fall_constraints_[row][col] = constr;
}

std::vector<std::vector<NanoSecond>>& Arc::GetFallConstraints()
{
    return fall_constraints_;
}

void Arc::AddSimulation(size_t row, size_t col, Simulation *simulation)
{
    ENLARGE_MATRIX(simulations_, std::vector<Simulation*>, row + 1, col + 1,
                    std::vector<Simulation*>());
    simulations_[row][col].push_back(simulation);
}

std::vector<std::vector<std::vector<Simulation*>>>& Arc::GetSimulations()
{
    return simulations_;
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
    switch (kind_) {
    case ArcKind::COMBO:
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
    case ArcKind::SEQ_CK:
    {
        return pin_->cell_->GetSequential().GetClockPin();
    }
    default:
        assert(false);
    }

    return nullptr;
}

Template* Arc::GetTemplate()
{
    return template_;
}

UnateKind Arc::GetUnateness()
{
    switch (kind_) {
    case ArcKind::COMBO:
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
                return UnateKind::POSITIVE_UNATE;
            } else {
                return UnateKind::NEGATIVE_UNATE;
            }
        } else {
            if (out_a_ == 0 && out_b_ == 1) {
                return UnateKind::NEGATIVE_UNATE;
            } else {
                return UnateKind::POSITIVE_UNATE;
            }
        }
    }

    default:
        return UnateKind::NON_UNATE;
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

    TAB_FPRINTF(tab, f, "timing_type : ");
    switch (kind_) {
    case ArcKind::COMBO:
        fprintf(f, "combinational ;\n");
        break;
    case ArcKind::SEQ_CK:
    {
        EdgeKind kind = pin_->cell_->GetSequential().GetClockPolarity();
        if (kind == EdgeKind::RISING) {
            fprintf(f, "rising_edge ;\n");
        } else {
            fprintf(f, "falling_edge ;\n");
        }
        break;
    }
    case ArcKind::SEQ_CLR:
        fprintf(f, "clear ;\n");
        break;
    case ArcKind::SEQ_SET:
        fprintf(f, "set ;\n");
        break;
    }

    TAB_FPRINTF(tab, f, "timing_sense : ");
    switch (GetUnateness()) {
    case UnateKind::POSITIVE_UNATE:
        fprintf(f, "positive_unate ;\n");
        break;
    case UnateKind::NEGATIVE_UNATE:
        fprintf(f, "negative_unate ;\n");
        break;
    case UnateKind::NON_UNATE:
        fprintf(f, "non_unate ;\n");
        break;
    }

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

    if (rise_powers_.size() > 0 || fall_powers_.size() > 0) {
        TAB_FPRINTF(tab, f, "internal_power () {\n");
        tab++;

        TAB_FPRINTF(tab, f, "related_pin : %s ;\n", rel_pin->name_);

        if (rise_powers_.size() > 0) {
            WriteTable(f, tab, rise_powers_, "rise_power");
        }

        if (fall_powers_.size() > 0) {
            WriteTable(f, tab, fall_powers_, "fall_power");
        }

        tab--;
        TAB_FPRINTF(tab, f, "}\n");
    }

}

}
