

#include "DelayTable.h"

namespace open_char {

DelayTable::DelayTable()
{}

void DelayTable::Print()
{
    size_t width = delay_[0].size() * 10 + 1;

    PRINT_LINE(width)

    printf("Inputs from: %08b\n", in_from_);
    printf("Inputs to:   %08b\n", in_to_);
    printf("Output from: %8b\n", out_from_);
    printf("Output to:   %8b\n", out_to_);

    PRINT_LINE(width)

    for (const auto &row : delay_) {
        printf("|");
        for (const auto & cell: row) {
            printf(" %7.5f |", cell);
        }
        printf("\n");
    }

    PRINT_LINE(width)
}

}
