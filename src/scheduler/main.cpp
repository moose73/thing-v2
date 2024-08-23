#include <Arduino.h>
#include "simplescheduler.h"
#include "peripherals.h"
#include "alti_handler.h"
#include "battery_monitor.h"
#include "display_handler.h"
#include "idle_state.h"
#include "flash.h"
#include "wifi.h"

void setup() {

    // delay(1000);

    Serial.begin(115200);

    init_scheduler();

    setup_peripherals();

    initialize_altimeters();
    setup_battery();
    setup_display();
    init_flash(get_spi_bus());

    init_wifi();

    #ifdef ERASE_FLASH
    while (!Serial);
    Serial.printf("Confirming flash erase?\n");
    while (!Serial.available());
    Serial.printf("erasing...");
    erase_flash();
    Serial.printf("erase done");
    while (1);
    #endif

    #ifdef DUMP_FLASH_TO_SMON
    while (!Serial);
    // for (int i = 0; i < 20; i++) {
        // Serial.printf("flight %d\n", i);
        while (true) {
            if (Serial.available()) {
                while (Serial.read() != '\n');
                print_flight_to_smon(9);
            }
        }
        // print_flight_to_smon(9);
    // }

    while (1);
    #endif

    begin_idle_state();
    
    start_scheduler();



}

void loop() {};