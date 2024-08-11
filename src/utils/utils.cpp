#include "utils.h"

uint32_t simulate_flight_alti() {
    uint32_t time = millis();

    uint32_t delay_time = 10e3;
    uint32_t ascent_time = 20e3;
    uint32_t peak_alti = 20e3;
    uint32_t descent_time = 5e3;

    if (time < delay_time) return 0;

    if (time < (ascent_time + delay_time)) {
        return 15 + ((time - delay_time) * (peak_alti / ascent_time));
    } else if (time < (ascent_time + delay_time + descent_time)) {
        return 15 + peak_alti - ((time - ascent_time - delay_time) * (peak_alti / descent_time));
    } else {
        return 15;
    }
}