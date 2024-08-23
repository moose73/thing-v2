#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include "simplescheduler.h"
#include "peripherals.h"
#include "pins.h"
#include <TFT_eSPI.h>
#include <FT6236.h>
#include "battery_monitor.h"
#include "alti_handler.h"
#include "utils/utils.h"
#include "bitmaps.h"
#include "idle_state.h"
#include "wifi.h"



#pragma once


void setup_display();
uint32_t update_battery_stats();
uint32_t update_altimeter_display();
uint32_t update_wifi_rssi_bars();


#endif

