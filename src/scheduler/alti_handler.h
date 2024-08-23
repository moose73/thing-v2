#include <Arduino.h>
#include "simplescheduler.h"
#include <SPI.h>
// #include <LPS22HHSensor.h>
#include "lps22hh_lite.h"
#include "pins.h"
#include "peripherals.h"
#include "utils/utils.h"
#include "FIRFilter.h"
#include "IIRFilter.h"
#include "flash.h"


void initialize_altimeters();
void set_alti_zero(float alti);
float get_alti_zero();

float get_altitude_1();
float get_display_alti();

float get_vspeed_1();