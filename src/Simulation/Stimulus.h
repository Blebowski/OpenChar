
#ifndef STIMULUS_H
#define STIMULUS_H

#include <vector>
#include <string>

#include "open_char.h"

class open_char::Stimulus {

    StimulusKind kind;

    union val {
        struct {
            double v1;
            double v2;

            // All in femto-seconds
            double t_delay;
            double t_rise;
            double t_fall;
            double pulse_width;
            double period;
            double num_pulses;
        } pulse;
        double constant;
    };

};

#endif