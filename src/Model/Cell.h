
#ifndef CELL_H
#define CELL_H

#include <string>
#include <map>
#include <ranges>
#include <vector>

#include "open_char.h"
#include "Sequential.h"
#include "Pin.h"

namespace open_char {

class Cell {

    public:
        Cell(std::string name, Library *library);
        ~Cell();

        Library* GetLibrary();
        const std::string& GetName();

        CellKind GetKind();
        void SetKind(CellKind kind);

        std::pair<Pin&, bool> AddPin(std::string name, PinDirection direction, PinKind kind);
        Pin& GetPin(std::string name);

        void AddLeakageTableEntry(Expression *e, NanoWatt pwr);

        auto GetPins() {
            return std::views::values(pins_);
        };

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

        auto GetPins(PinDirection direction, PinKind kind) {
            auto filtered = std::views::filter(pins_,
                [kind, direction](const auto& pair) {
                    return (pair.second.kind_ == kind) && (pair.second.direction_ == direction);
                }
            );
            return std::views::values(filtered);
        };

        size_t GetPinsCount(PinDirection direction);

        Sequential& GetSequential();

        void SetArea(double area);
        double GetArea();

        void SetFootprint(std::string footprint);
        std::string &GetFootPrint();

        Template* GetDelayTemplate();
        void SetDelayTemplate(Template *d_template);

        Template* GetConstraintTemplate();
        void SetConstraintTemplate(Template *c_template);

        void AddSimulation(Simulation *simulation);
        std::vector<Simulation*>& GetSimulations();
        bool IsSimulationFinished();

        void SetCharactState(CharactState charact_state);
        CharactState GetCharactState();

        void WriteLiberty(FILE *f, size_t tab);
        void WriteVerilog(FILE *f);

    private:
        const std::string name_;
        Library* library_;
        std::map<std::string, Pin> pins_;
        Template *d_template_;
        Template *c_template_;
        CellKind kind_;
        double area_;
        std::string footprint_;

        // First value - Cell inputs
        // Second value - Leakage power upon such state
        std::vector<std::pair<Expression*, NanoWatt>> leakage_table_;

        Sequential seq_;
        bool has_seq_;

        std::vector<Simulation*> simulations_;
        CharactState charact_state_;
};

}

#endif