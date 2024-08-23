#include "lps22hh_lite.h"

LPS22HH::LPS22HH(SPIClass *spi_obj, uint8_t cs_pin) {
    spi = spi_obj;
    cs = cs_pin;
}

void LPS22HH::init() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    // Serial.printf("Reg WHOAMI is value %d\n", read_reg8(0x0F));

    assert (read_reg8(0x0F) == 0xB3);
    write_reg8(0x11, 0b00000100);
    delay(1000);
    write_reg8(0x0E, 0b00000011);
    write_reg8(0x10, 0b0 | (0b101 << 4) | (0b10 << 2) | (0b00)); //75hz
    write_reg8(0x11, 0b00010010);

    write_reg8(0x13, 0b00000111);

}

uint8_t LPS22HH::read_reg8(uint8_t addr) {
    digitalWrite(cs, LOW);
    spi->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    spi->transfer(addr | 0x80);
    uint8_t val = spi->transfer(0x00);
    spi->endTransaction();
    digitalWrite(cs, HIGH);
    return val;
}

void LPS22HH::write_reg8(uint8_t addr, uint8_t val) {
    digitalWrite(cs, LOW);
    spi->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    spi->transfer(0x7F & addr);
    spi->transfer(val);
    spi->endTransaction();
    digitalWrite(cs, HIGH);
}

uint8_t LPS22HH::get_num_fifo() {
    return read_reg8(0x25);
}

float LPS22HH::get_pressure() {
    uint32_t press = read_reg8(0x7A)<<16 | read_reg8(0x79)<<8 | read_reg8(0x78);
    uint32_t temp = read_reg8(0x7C)<<8 | read_reg8(0x7B);
    return press / 4096.0;  
}