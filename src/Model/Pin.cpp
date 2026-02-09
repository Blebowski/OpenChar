
#include "open_char.h"
#include "Pin.h"
#include "Cell.h"
#include "Utils.h"

namespace open_char {

Pin::Pin(Cell *cell, std::string name, PinDirection direction, PinKind kind) :
    cell_(cell),
    name_(name),
    direction_(direction),
    kind_(kind)
{}

Pin::Pin() :
    name_(""),
    direction_(PinDirection::IN),
    kind_(PinKind::DATA)
{}

void Pin::AddLogicTableEntry(int64_t inputs, int output)
{
    logic_table_.push_back(std::make_pair(inputs, output));
}

std::pair<int64_t, int> Pin::GetLogicTableEntry(int index)
{
    return logic_table_[index];
}


const std::vector<std::pair<int64_t, int>>& Pin::GetLogicTable()
{
    return logic_table_;
}

void Pin::SetDelayTable(DelayTable &&delay_table)
{
    delay_table_ = delay_table;
}

void Pin::PrintLogicTable()
{
    size_t i_pins_cnt = cell_->GetPinsCount(PinDirection::IN);
    size_t line_len = (i_pins_cnt + 1) * 10 + 1;

    PRINT_LINE(line_len)

    printf("|");
    for (const auto &i_pin : cell_->GetPins(PinDirection::IN)) {
        printf(" %7s |", i_pin.name_);
    }
    printf(" %7s |\n", name_);

    PRINT_LINE(line_len)

    for (const auto &v : logic_table_) {
        printf("|");

        int64_t tmp = v.first;
        for (size_t i = 0; i < i_pins_cnt; i++) {
            printf(" %7d |", tmp & 0x1);
            tmp >>= 1;
        }

        printf(" %7d |\n", v.second & 0x1);
    }

    PRINT_LINE(line_len)
}

}