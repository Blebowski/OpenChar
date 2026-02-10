
#ifndef CELL_H
#define CELL_H

#include <string>
#include <map>
#include <ranges>

#include "Pin.h"

namespace open_char {

class Cell {

    public:
        Cell(std::string name);

        Library* lib_;
        const std::string name_;

        std::pair<Pin&, bool> AddPin(std::string name, PinDirection direction, PinKind kind);
        std::map<std::string, Pin>& GetPins();

        Pin& GetPin(std::string name);

        auto GetPins(PinDirection direction) {
            auto filtered = std::views::filter(pins_,
                [direction](const auto& pair) {
                    return pair.second.direction_ == direction;
                }
            );
            return std::views::values(filtered);
        };

        auto GetPins(PinKind kind) {
            auto filtered = std::views::filter(pins_,
                [kind](const auto& pair) {
                    return pair.second.kind_ == kind;
                }
            );
            return std::views::values(filtered);
        };

        size_t GetPinsCount(PinDirection direction);

        Template* GetDelayTemplate();
        void SetDelayTemplate(Template *d_template);

        void WriteLiberty(FILE *f, size_t tab);

    private:
        std::map<std::string, Pin> pins_;
        Template *d_template_;

};

}

#endif