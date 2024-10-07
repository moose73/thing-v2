#include <Arduino.h>
#pragma once

float simulate_flight_alti();

typedef enum  {
    GROUND,
    FLIGHT,
    FREEFALL,
    CANOPY
} State ;


#if defined(PIC1)
#define START_MEDIA 300
#elif defined(PIC2)
#define START_MEDIA 480
#else
#define START_MEDIA 500
#endif
#define min(a,b) ((a) < (b) ? (a) : (b))


