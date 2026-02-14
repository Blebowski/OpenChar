
#include "open_char.h"

#include "Pin.h"
#include "Cell.h"
#include "Library.h"
#include "Utils.h"

namespace open_char {

Pin::Pin(Cell *cell, std::string name, PinDirection direction, PinKind kind) :
    cell_(cell),
    name_(name),
    direction_(direction),
    kind_(kind),
    func_(nullptr)
{}

Pin::Pin() :
    cell_(nullptr),
    name_(""),
    direction_(PinDirection::IN),
    kind_(PinKind::DATA),
    func_(nullptr)
{}

Pin::~Pin()
{
    if (func_ != nullptr)
        delete func_;
}

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

void Pin::AddDelayTable(DelayTable delay_table)
{
    delay_tables_.push_back(delay_table);
}

std::vector<DelayTable>& Pin::GetDelayTables()
{
    return delay_tables_;
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

void Pin::SetLogicFunction(Expression *e)
{
    func_ = e;
}

Expression *Pin::GetLogicFunction()
{
    assert (func_ != nullptr);
    return func_;
}

void Pin::PrintLogicFunction()
{
    printf("%s: ", name_);
    func_->Print();
    printf("\n");
}

void Pin::WriteLiberty(FILE *f, size_t tab)
{
    Supply *supply = cell_->GetLibrary()->GetOpCond().GetSupply();

    switch (kind_) {
    case PinKind::PWR:
        TAB_FPRINTF(tab, f, "pg_pin (%s) {\n", name_);
        tab++;
        TAB_FPRINTF(tab, f, "voltage_name : %s ;\n", supply->GetVddName());
        TAB_FPRINTF(tab, f, "pg_type : primary_power ;\n");
        tab--;
        TAB_FPRINTF(tab, f, "} /* end pg_pin */\n");
        break;

    case PinKind::GND:
        TAB_FPRINTF(tab, f, "pg_pin (%s) {\n", name_);
        tab++;
        TAB_FPRINTF(tab, f, "voltage_name : %s ;\n", supply->GetGndName());
        TAB_FPRINTF(tab, f, "pg_type : primary_ground ;\n");
        tab--;
        TAB_FPRINTF(tab, f, "} /* end pg_pin */\n");
        break;

    case PinKind::DATA:
        TAB_FPRINTF(tab, f, "pin (%s) {\n", name_);
        tab++;

        switch (direction_) {
        case PinDirection::IN:
            TAB_FPRINTF(tab, f, "direction : input ;\n");
            break;
        case PinDirection::OUT:
            TAB_FPRINTF(tab, f, "direction : output ;\n");
            break;
        default:
            // TODO: Introduce printable enum class ?
            error("Unhandled pin_direction\n");
        }

        TAB_FPRINTF(tab, f, "capacitance : 0.0 ;\n");

        if (direction_ == PinDirection::OUT) {
            for (auto & delay_table : delay_tables_) {
                delay_table.WriteLiberty(f, tab);
            }
        }

        tab--;
        TAB_FPRINTF(tab, f, "} /* end pin */\n");
        break;

    default:
        break;
    }
}

}
