
#pragma once
#include <Arduino.h>
#include "simplescheduler.h"
#include <WiFi.h>
#include "config.h"
#include "utils/utils.h"
#include <esp_wifi.h>
#include "esp_http_client.h"
// #include <HTTPClient.h>


void init_wifi();

uint32_t reconnect_wifi();
uint8_t get_rssi_bars();

void set_time();

typedef struct {
    String ssid;
    String password;
} WiFi_AP;