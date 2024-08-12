#include <Arduino.h>
#include "flash_record/flash_record.h"

void setup() {
    Serial.begin(1000000);
    Serial.println("dumping flash");

    init_flash();


}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'd') {
            dump_data();
        }
    }
}