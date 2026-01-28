#include <fmt/printf.h>

#include "open_char.h"
#include "Pin.h"
#include "Cell.h"

namespace open_char {

#define PRINT_LINE(len) fmt::printf("%s\n", std::string(len, '-'));

Pin::Pin(Cell *cell, std::string name, PinDirection direction, PinKind kind) :
    cell_(cell),
    name_(name),
    direction_(direction),
    kind_(kind)
{}

void Pin::AddLogTableEntry(int64_t inputs, int output)
{
    int64_t v = inputs | (((int64_t)output) << 63);
    logic_table_.push_back(v);
}

void Pin::PrintLogicTable()
{
    size_t i_pins_cnt = cell_->GetPinsCount(PinDirection::IN);
    size_t line_len = (i_pins_cnt + 1) * 10 + 1;

    PRINT_LINE(line_len)


    fmt::printf("|");
    for (const auto &i_pin : cell_->GetPins(PinDirection::IN)) {
        fmt::printf(" %7s |", i_pin.name_);
    }
    fmt::printf(" %7s |\n", name_);

    PRINT_LINE(line_len)

    for (const auto &v : logic_table_) {
        fmt::printf("|");

        int64_t tmp = v;
        for (size_t i = 0; i < i_pins_cnt; i++) {
            fmt::printf(" %7d |", tmp & 0x1);
            tmp >>= 1;
        }

        fmt::printf(" %7d |\n", (v >> 63) & 0x1);
    }

    PRINT_LINE(line_len)
}

}