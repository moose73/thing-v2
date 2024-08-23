#include "peripherals.h"


SPIClass spi_bus = SPIClass(HSPI);

void setup_peripherals() {
    Serial.printf("spi bus addr is %x\n", &spi_bus);
    spi_bus.begin(SPI_CLK, SPI_IO1, SPI_IO0);
    Wire.begin(1, 2);
}

SPIClass* get_spi_bus() {
    return &spi_bus;
}