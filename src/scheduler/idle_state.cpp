#include "idle_state.h"
#include "display_handler.h"

State current_state = GROUND;

void begin_idle_state(void) {

  set_alti_zero(get_alti_zero());

  add_task(update_battery_stats, 0);
  add_task(update_altimeter_display, 0);
  add_task(update_state_idle, 0);
  add_task(zero_altis_ground, 0);
  add_task(update_wifi_rssi_bars, 0);
}

uint32_t first_time_above_500ft = 0;
uint32_t first_time_down_vspeed_gt_50 = 0;
float flight_max_alti = 0;

uint32_t first_time_vertical_speed_close_to_zero_low = 0;

uint32_t update_state_idle() {

  float altitude = get_display_alti();
  // Serial.printf("current state is %d. alti is %f\n", current_state,
  // altitude);
  // Serial.printf("Start state is %d, alti is %f\n", current_state, altitude);

  // above 100 ft for 2 seconds, go to FLIGHT state
  if (altitude > 500 && (first_time_above_500ft == 0)) {
    first_time_above_500ft = millis();
  } else if (altitude < 500) {
    first_time_above_500ft = 0;
  }
  if (current_state == GROUND && altitude > 500 && (millis() - first_time_above_500ft) > 2e3) {
    current_state = FLIGHT;
  } 

  if (current_state == FLIGHT && altitude > flight_max_alti) {
    // Serial.printf("setting max alti to %f\n", altitude);
    flight_max_alti = altitude;
  }

  if (current_state == FLIGHT && (altitude < (flight_max_alti - 300)) && (get_vspeed_1() < -50)) {
    current_state = FREEFALL;
  }

  if (current_state == FLIGHT && (altitude < (flight_max_alti - 1500))) {
    current_state = CANOPY;
  }

  if ((current_state == FREEFALL) && (altitude < (flight_max_alti - 1000)) && (abs(get_vspeed_1()) < 10)) {
    current_state = CANOPY;
  }

  // Serial.printf("vspeed now is %f, ftvsctzl %d, millis %d\n", get_vspeed_1(), first_time_vertical_speed_close_to_zero_low, millis());
  if (altitude < 1000 &&  abs(get_vspeed_1()) < 8) {
    if (first_time_vertical_speed_close_to_zero_low == 0) {
      first_time_vertical_speed_close_to_zero_low = millis();
    } else if (
      (millis() - first_time_vertical_speed_close_to_zero_low) > 5000) {
      current_state = GROUND;
      flight_max_alti = 0;
    } else if (millis() - first_time_vertical_speed_close_to_zero_low > 10000) {
      first_time_vertical_speed_close_to_zero_low = 0;
    }
  } else {
    first_time_vertical_speed_close_to_zero_low = 0;
  }

  // Serial.printf("Current state is %d, alti is %f, max alti is %f\n", current_state, altitude, flight_max_alti);


  // if (altitude > 500 && (first_time_above_500ft > 0) &&
  //     (millis() - first_time_above_500ft) > 2e3) {
  //   current_state = FLIGHT;
  // } else if (altitude < 500) {
  //   first_time_above_500ft = 0;
  //   current_state = GROUND;
  // }

  return 100 * 1000;
}

State get_current_state(void) { return current_state; }

float one_back_alti = 0;
uint32_t zero_altis_ground() {
  // Serial.printf("millis is %d\n", millis());

  if (one_back_alti == 0) {
    one_back_alti = get_alti_zero();
    return 120 * 1000 * 1000;
  }

  if (get_display_alti() < 100) {
    float current_zero_alti = get_alti_zero();
    Serial.printf("current is %f, one back alti is %f, alti is %f\n",
                  current_zero_alti, one_back_alti, get_display_alti());
    Serial.printf("zeroing to %f\n", one_back_alti);
    set_alti_zero(one_back_alti);
    one_back_alti = current_zero_alti;
  }
  return 120 * 1000 * 1000;
}