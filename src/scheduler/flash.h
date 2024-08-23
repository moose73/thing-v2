#include "Arduino.h"
#include <SPI.h>
#include <SPIFlash.h>
#include "pins.h"
#include "idle_state.h"
#include "utils/utils.h"

void init_flash(SPIClass *spi);

void erase_flash();

void flash_process_altitude(float alti);

void print_flight_to_smon(int flight_num);