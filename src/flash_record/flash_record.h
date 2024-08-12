#include "pins.h"
#include <SPIFlash.h>
#include <SPI.h>
#include <Arduino.h>

void init_flash();
bool process_gross_singlerec(float alti, float time);
void dump_data();