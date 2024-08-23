#include <Arduino.h>
#include <SPI.h>
#include "pins.h"
#include "lps22hh_lite.h"

SPIClass spi_bus = SPIClass(HSPI);
LPS22HH sensor(&spi_bus, SPI_LPCS);

void setup() {
    delay(5000);
    Serial.begin(115200);
    spi_bus.begin(SPI_CLK, SPI_IO1, SPI_IO0);

    Serial.printf("done initing spi bus\n");

    sensor.init();


    
}

void loop() {
    delay(1000);
    Serial.printf("num fifo is %d\n", sensor.get_num_fifo());
    for (int i = 0; i < 40; i++) {
        if (sensor.get_num_fifo() > 0) {
            Serial.printf("pressure is %f\n", sensor.get_pressure());
            Serial.printf("fifo is %d\n", sensor.get_num_fifo());
        }
    }
}