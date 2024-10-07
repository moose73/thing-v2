#include "utils.h"

bool simulated = false;
float simulate_flight_alti() {

  if (simulated) return 0;

  float time = millis();

  float delay_time = 10e3 ;
  float ascent_time = 10e3;
  float peak_alti = 8000;
  float descent_time = 20e3;
  float deploy_alti = 200;
  float canopy_time = 10e3;

//   if (millis() > 31000 && millis() < delay_time) {
//     return random(0, 5) + 30;
//   }
  if (time < delay_time)
    return 0;
  if (time < (ascent_time + delay_time)) {

    uint32_t outval = 0 + ((time - delay_time) * (peak_alti / ascent_time));
    return random(0, 5) + outval;

  } else if (time < (ascent_time + delay_time + descent_time)) {
    return random(0, 5) + 0 + peak_alti -
           ((time - ascent_time - delay_time) * ((peak_alti-deploy_alti) / descent_time));
  } else if (time < (ascent_time+delay_time+descent_time+canopy_time)) {
    return deploy_alti - (random(0, 5) + (deploy_alti * (time - ascent_time - delay_time - descent_time) / canopy_time));
  } else {
    simulated = true;
    return 0;
  }
}