
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
        double volage_;

        // Pulse source
        double v1_;
        double v2_;

        // All in femto-seconds
        double t_delay_;
        double t_rise_;
        double t_fall_;
        double pulse_width_;
        double period_;
        double num_pulses_;

        Stimulus(double voltage);
        Stimulus(double v1, double v2, double t_delay, double t_rise, double t_fall,
                 double pulse_width, double period, double num_pulses);
};

}

#endif