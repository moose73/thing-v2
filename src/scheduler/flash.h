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

void erase_video_space();
void write_video_data(uint16_t data);
void flash_read_bytes(uint32_t addr, uint8_t *buf, uint32_t len);