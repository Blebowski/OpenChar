

#include "Cell.h"
#include "DelayTable.h"
#include "Pin.h"
#include "Template.h"
#include "Utils.h"

namespace open_char {

DelayTable::DelayTable(Pin *pin, Template *templ, int64_t in_from,
                       int64_t in_to, int out_from, int out_to) :
    pin_(pin),
    template_(templ),
    in_from_(in_from),
    in_to_(in_to),
    out_from_(out_from),
    out_to_(out_to)
{}

void DelayTable::AddDelay(size_t row, NanoSecond delay)
{
    while (delays_.size() <= row)
        delays_.push_back(std::vector<NanoSecond>());
    delays_[row].push_back(delay);
}

std::vector<std::vector<NanoSecond>>& DelayTable::GetDelays()
{
    return delays_;
}

void DelayTable::Print()
{
    size_t width = delays_[0].size() * 10 + 1;

    PRINT_LINE(width)

    printf("Inputs from: %08b\n", in_from_);
    printf("Inputs to:   %08b\n", in_to_);
    printf("Output from: %8b\n", out_from_);
    printf("Output to:   %8b\n", out_to_);

    PRINT_LINE(width)

    for (const auto &row : delays_) {
        printf("|");
        for (const auto & cell: row) {
            printf(" %7.5f |", cell);
        }
        printf("\n");
    }

    PRINT_LINE(width)
}

std::pair<Pin *, EdgeKind> DelayTable::GetRelatedPin()
{
    int64_t diff = in_from_ ^ in_to_;

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

    EdgeKind kind = ((in_to_ >> pos) & 0x1) ? EdgeKind::RISING : EdgeKind::FALLING;

    return {rel_pin, kind};
}

void DelayTable::WriteLiberty(FILE *f, size_t tab)
{
    TAB_FPRINTF(tab, f, "timing () {\n");
    tab++;

    std::pair<Pin *, EdgeKind> pp = GetRelatedPin();
    TAB_FPRINTF(tab, f, "related_pin : %s ;\n", pp.first->name_);

    if (pp.second == EdgeKind::RISING) {
        TAB_FPRINTF(tab, f, "cell_rise() {\n");
    } else {
        TAB_FPRINTF(tab, f, "cell_fall() {\n");
    }

    tab++;

    // TODO: Move to the template !
    TAB_FPRINTF(tab, f, "index_1 (\"");

    size_t i = 0;
    for (const auto & v : template_->index_1_) {
        fprintf(f, "%f%s", v, (i < template_->index_1_.size() - 1) ? ", " : "");
        i++;
    }
    fprintf(f, "\")\n");

    TAB_FPRINTF(tab, f, "index_2 (\"");
    i = 0;
    for (const auto & v : template_->index_2_) {
        fprintf(f, "%f%s", v, (i < template_->index_2_.size() - 1) ? ", " : "");
        i++;
    }
    fprintf(f, "\")\n");

    TAB_FPRINTF(tab, f, "values ( \\\n");
    tab++;

    for (const auto & row : delays_) {
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

    if (pp.second == EdgeKind::RISING) {
        TAB_FPRINTF(tab, f, "} /* end cell_rise */\n");
    } else {
        TAB_FPRINTF(tab, f, "} /* end cell_fall */\n");
    }

    tab--;
    TAB_FPRINTF(tab, f, "} /* end timing */\n");
}

}
