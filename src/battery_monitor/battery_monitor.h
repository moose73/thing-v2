#include <SparkFunBQ27441.h>
#include "TFT_eSPI.h"


void setupBQ27441(void);

void printBatteryStats();

uint32_t update_battery_display(TFT_eSPI tft);

const unsigned int BATTERY_CAPACITY = 2000; 