#include "utils.h"

float simulate_flight_alti() {
    float time = millis();

    float delay_time = 10e3;
    float ascent_time = 100e3;
    float peak_alti = 5000;
    float descent_time = 20e3;

    if (time < delay_time) return 0;
    if (time < (ascent_time + delay_time)) {
        
        uint32_t outval = 15 + ((time - delay_time) * (peak_alti / ascent_time));
        return random(0, 20) + outval;
        
    } else if (time < (ascent_time + delay_time + descent_time)) {
        return random(0, 20) + 15 + peak_alti - ((time - ascent_time - delay_time) * (peak_alti / descent_time));
    } else {
        return random(0, 10) + 15;
    }
}