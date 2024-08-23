#include <Arduino.h>
#include <SPI.h>
#include "pins.h"
#include <Wire.h>

void setup_peripherals();
SPIClass* get_spi_bus();