
#ifndef STIMULUS_H
#define STIMULUS_H

#include <vector>
#include <string>

#include "open_char.h"

namespace open_char {

class Stimulus {

    public:
        StimulusKind kind_;

        // Constant source
        Volt volage_;

        // Pulse source
        Volt v1_;
        Volt v2_;

        // All in femto-seconds
        NanoSecond t_delay_;
        NanoSecond t_rise_;
        NanoSecond t_fall_;
        NanoSecond pulse_width_;
        NanoSecond period_;
        int num_pulses_;

        Stimulus(Volt voltage);
        Stimulus(Volt v1, Volt v2, NanoSecond t_delay, NanoSecond t_rise,
                 NanoSecond t_fall, NanoSecond pulse_width, NanoSecond period,
                 int num_pulses);
};

}

#endif