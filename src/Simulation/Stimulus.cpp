#include <vector>
#include <string>

#include "open_char.h"
#include "Stimulus.h"

namespace open_char {

    Stimulus::Stimulus(Volt voltage) :
        kind_(StimulusKind::CONSTANT),
        volage_(voltage)
    {};

    Stimulus::Stimulus(Volt v1, Volt v2, NanoSecond t_delay, NanoSecond t_rise,
                       NanoSecond t_fall, NanoSecond pulse_width, NanoSecond period,
                       int num_pulses):
        kind_(StimulusKind::PULSE),
        v1_(v1),
        v2_(v2),
        t_delay_(t_delay),
        t_rise_(t_rise),
        t_fall_(t_fall),
        pulse_width_(pulse_width),
        period_(period),
        num_pulses_(num_pulses)
    {};

}
