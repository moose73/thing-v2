#include <SPI.h>
#include <Arduino.h>
#pragma once

class LPS22HH {

    SPIClass* spi;
    uint8_t cs;
    uint8_t read_reg8(uint8_t addr);
    void write_reg8(uint8_t addr, uint8_t val);

public:
    LPS22HH(SPIClass *spi_obj, uint8_t cs_pin);

    void init();
    uint8_t get_num_fifo();
    float get_pressure();

    
};