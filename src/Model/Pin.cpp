#include "open_char.h"
#include "Pin.h"

namespace open_char {

Pin::Pin(std::string name, PinDirection direction, PinKind kind) :
    name_(name),
    direction_(direction),
    kind_(kind)
{}

}