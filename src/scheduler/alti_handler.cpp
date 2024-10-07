#include "alti_handler.h"
float alti_1_offs = 0;
float alti_2_offs = 0;

SPIClass *spi = NULL;
// LPS22HHSensor *alti_1 = NULL;
LPS22HH *alti_1 = NULL;


float alti_inner_loop_freq = 75;

uint32_t collect_altitude();

// const double b_coeffs_vspeed[] = {0.5, 0.5};
// const double a_coeffs_vspeed[] = {1, 0};
// const double a_coeffs[] = {1, -3.12623643,  3.74059895, -2.02049898,  0.41456739};
// IIRFilter vspeed_filter(b_coeffs, a_coeffs);
// IIRFilter vspeed_filter(b_coeffs_vspeed, a_coeffs_vspeed);

const double b_coeffs_alti[] = {0.16203149, -0.10639242, -0.10639242,  0.16203149};
const double a_coeffs_alti[] = { 1, -2.12701923,  1.67254392, -0.43424654};
IIRFilter altitude_filter(b_coeffs_alti, a_coeffs_alti);

void initialize_altimeters() {
  spi = get_spi_bus();
  alti_1 = new LPS22HH(spi, SPI_LPCS);

  alti_1->init();

  add_task(collect_altitude, 0);
}

float pressure_to_altitude(float pressure) {
  return 44330 * (1.0 - pow(pressure / 1013.25, 0.1903)) * 3.28;
}



float prev_display_alti = 0;
float vspeed = 0;
float get_vspeed_1() {
  return vspeed;
}


float last_altitude = 0;
float last_vspeed = 0;

int vspeed_ds_ctr = 0;
float alti_old_vspeed = 0;
int vspeed_ds_factor = 35;

int taskctr = 0;



void make_alti_measurement() {

    taskctr++;
    vspeed_ds_ctr++;
    float pressure = alti_1->get_pressure();
    #ifdef SIM_FLIGHT
    float altitude = pressure_to_altitude(pressure) - alti_1_offs + simulate_flight_alti(); 
    #else
    float altitude = pressure_to_altitude(pressure) - alti_1_offs;
    #endif
    // vspeed = vspeed_filter.filter((altitude - prev_display_alti) * alti_inner_loop_freq);
    float ogalt = altitude;
    // altitude = altitude_filter.filter(altitude);
    // last_vspeed = vspeed;
    if (vspeed_ds_ctr == vspeed_ds_factor) {
      vspeed = (altitude - alti_old_vspeed) * (alti_inner_loop_freq / vspeed_ds_factor);
      vspeed_ds_ctr = 0;
      alti_old_vspeed = altitude;
    // Serial.printf("vspeed is %f, alti is %f\n", vspeed, altitude);
      
    }

    // Serial.printf("unfilt %f, filt %f\n", ogalt, altitude);
    prev_display_alti = altitude;
    last_altitude = altitude;
    if (taskctr > 10 && (millis() % 1000) < 100) {
      // Serial.printf("taskctr: %d, millis is %d\n", taskctr, millis());
      taskctr = 0;
    }
    flash_process_altitude(altitude);
}

uint32_t collect_altitude() {

    while (alti_1->get_num_fifo() > 0) {
      make_alti_measurement();
    } 
    return int(1e6 / alti_inner_loop_freq);
}

float get_altitude_1() {
  return last_altitude;
}

float get_alti_zero() {
  float pressure;
  float avgbf = 0;
  for (int i = 0; i < 20; i++) {
    pressure = alti_1->get_pressure();
    delay(30);
    #ifdef SIM_FLIGHT
    avgbf += (pressure_to_altitude(pressure) + simulate_flight_alti());
    #else 
    avgbf += pressure_to_altitude(pressure);
    #endif
  }
  return avgbf / 20;
  // Serial.printf("alti 1 offset is %f\n", alti_1_offs);
}

void set_alti_zero(float alti) {
  alti_1_offs = alti;
}




float get_display_alti() { 

  return get_altitude_1();
  }