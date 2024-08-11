#include "flash_record.h"



SPIFlash flash(SPI_FLASHCS_PIN, 0);

void init_flash() {
    flash.initialize();
    flash.readUniqueId();

    Serial.printf("Byte at 0x67: %x\n", flash.readByte(0x67));
    // flash.writeByte(0x67, 0x69);


    
}