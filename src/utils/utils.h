#include <Arduino.h>
#pragma once

float simulate_flight_alti();

typedef enum  {
    GROUND,
    FLIGHT,
    FREEFALL,
    CANOPY
} State ;


