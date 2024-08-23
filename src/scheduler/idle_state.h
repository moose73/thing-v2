#include "simplescheduler.h"
#include <Arduino.h>
#include "peripherals.h"
#include "pins.h"
#include "display_handler.h"
#include "utils/utils.h"
#include <math.h>

#pragma once


void begin_idle_state();
uint32_t update_state_idle();
State get_current_state();
uint32_t zero_altis_ground();

