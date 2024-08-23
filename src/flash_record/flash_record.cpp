#include "flash_record.h"



SPIFlash flash(SPI_FLASHCS_PIN, 0);

void init_flash() {
    flash.initialize();
    flash.readUniqueId();
    for (int i = 0; i < 30; i++) {
        Serial.printf("Byte %d: %d\n", i, flash.readByte(i));
    }

}

bool rec_start = false;
uint32_t data_ptr = 0;

bool process_gross_singlerec(float alti, float time) {

    if (alti < 1000 && !rec_start) return false;

    if (!rec_start) {
        rec_start = true;
        for (int i = 0; i < 0; i++) {
            Serial.printf("Erasing block %d\n", i);
            flash.blockErase64K(i);
        }

    }
    flash.writeBytes(data_ptr, (uint8_t*)&time, 4);
    data_ptr += 4;
    flash.writeBytes(data_ptr, (uint8_t*)&alti, 4);
    data_ptr += 4;

    if (rec_start && alti < 100) {
        rec_start = false;
    }
    return true;
}

void dump_data() {
    for (int i = 0; i < 288000; i++) {
        Serial.printf("%x ", i, flash.readByte(i));

    }
}