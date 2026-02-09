
#include "Supply.h"

namespace open_char {

Supply::Supply(std::string vdd_name, Volt vdd_val, std::string gnd_name, Volt gnd_val) :
    vdd_name_(vdd_name),
    vdd_val_(vdd_val),
    gnd_name_(gnd_name),
    gnd_val_(gnd_val)
{}

}