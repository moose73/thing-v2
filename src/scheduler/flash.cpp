#include "flash.h"

SPIFlash flash(SPI_FLASHCS_PIN, 0);



/*
Flash structure:
- 32K pages
- first page is the map. Total of 500 pages (16MByte flash). Max of 256 flights. Only use first 300 pages: other 200 for other stuff.
    - 4 bytes per page. First byte (divis-by-2) is empty / used (0xFF / 0x00).  bytes 2-4 is flight number
- Recording is simple: 2-byte uint16_t altitudes. Easy peasy.
    
*/
void find_next_avlbl_sector();
uint16_t next_avlbl_sector = 0;
uint16_t next_avlbl_flight_num = 0;
#define REC_BUFFER_SIZE 500
uint16_t recording_buffer[REC_BUFFER_SIZE];
uint16_t recording_buffer_ptr = 0;
bool recording_to_flash = false;
bool sector_transition = false;
#define FLIGHT_MAX_PAGES 300

void init_flash(SPIClass *spi) {
    Serial.printf("Initializing flash\n");

    flash.initialize(spi);

    uint8_t buf[4];
    flash.readBytes(0, buf, 4);
    if (buf[0] == 0xFF) {
        Serial.printf("Flash is empty\n");
        buf[0] = 0x00; buf[1] = 0x00; buf[2] = 0x00; buf[3] = 0x00;
        flash.writeBytes(0, buf, 4);
    } else {
        Serial.printf("Flash is not empty\n");
    }

    find_next_avlbl_sector();

    //clear recording buffer
    for (int i = 0; i < REC_BUFFER_SIZE; i++) {
        recording_buffer[i] = 0;
    }


}

void find_next_avlbl_sector() {
    uint8_t buf[4];
    for (int i = 0; i < FLIGHT_MAX_PAGES; i++) {
        flash.readBytes(i * 4, buf, 4);
        if (buf[0] == 0xFF) {
            next_avlbl_sector = i;
            Serial.printf("Next available sector is sector %d. Next flight num is %d\n", next_avlbl_sector, next_avlbl_flight_num);
            
            return;
        } else {
                next_avlbl_flight_num = (buf[1] << 16) | (buf[2] << 8) | buf[3];
                if (!sector_transition) next_avlbl_flight_num += 1;
            
        }
    }
    Serial.printf("No available flights\n");
    next_avlbl_sector = FLIGHT_MAX_PAGES;
}

uint32_t write_ptr = 0;
uint32_t active_page_num = 0;

uint32_t write_flight_data(uint8_t *data, uint32_t len) {

    if ((len + write_ptr) > 32000) {
        uint32_t written = write_flight_data(data, 32000 - write_ptr);
        write_flight_data(data + written, len - written);
        sector_transition = true;
    } else {
        sector_transition = false;
    }

    if (active_page_num == 0 || (write_ptr == 32000)) {    
        uint8_t buf[4];
        flash.readBytes(next_avlbl_sector * 4, buf, 4);
        if (buf[0] != 0xFF) {
            Serial.printf("Error: flight %d is not available\n", next_avlbl_sector);
            return 0;
        }
        find_next_avlbl_sector();
        buf[0] = 0x00;
        buf[1] = (next_avlbl_flight_num & 0xFFFF00) >> 16;
        buf[2] = (next_avlbl_flight_num & 0xFF00) >> 8;
        buf[3] = next_avlbl_flight_num & 0xFF;
        flash.writeBytes(next_avlbl_sector * 4, buf, 4);
        active_page_num = next_avlbl_sector;
        write_ptr = 0;
    }

    flash.writeBytes((active_page_num * 32000) + write_ptr, data, len);
    // Serial.printf("Wrote %d bytes to page %d at offset %d\n", len, active_page_num, write_ptr);
    write_ptr += len;

    return len;

}

void print_flight_to_smon(int flight_num) {
    //first find how many pages this flight takes
    uint8_t buf[4];
    uint8_t start_page = 1;
    //search thru the page map (at the beginning) for the right flight
    for (int i = 0; i < FLIGHT_MAX_PAGES; i++) {
        flash.readBytes(i * 4, buf, 4);
        if (buf[0] == 0x00 && buf[2] == (flight_num & 0xFF00) >> 8 && buf[3] == (flight_num & 0xFF)) {
            start_page = i;
            break;
        }
    }
    uint8_t stop_page = start_page;
    for (int i = start_page; i < FLIGHT_MAX_PAGES; i++) {
        flash.readBytes(i * 4, buf, 4);
        flash.readBytes(i * 4, buf, 4);
        if (buf[0] == 0x00 && buf[2] == (flight_num & 0xFF00) >> 8 && buf[3] == (flight_num & 0xFF)) {
            stop_page += 1;
        }
    }

    // Serial.printf("Start is %d, stop is %d\n", start_page, stop_page);
    for (int i = start_page; i < stop_page; i++) {
        for (int j = 0; j < 32000; j += 2) {
            uint16_t alti;
            flash.readBytes((i * 32000) + j, (uint8_t *)&alti, 2);

            if (alti == 0xFFFF) {
                return;
            }
            Serial.printf("%d\n", alti);
            delay(10);
        }
    }

}

void erase_flash() {
    Serial.printf("Erasing flash\n");
    flash.chipErase();
    while (flash.busy());
}


void flash_process_altitude(float alti) {
    if (get_current_state() != FREEFALL) {
        if (recording_to_flash) {
            recording_to_flash = false;
            // stop recording, clear buffer
            active_page_num = 0;
            write_ptr = 0;
            for (int i = 0; i < REC_BUFFER_SIZE; i++) {
                recording_buffer[i] = 0;
            }
            // next_avlbl_flight_num += 1;

            Serial.printf("stopped recording\n");
            return;
        } 
        recording_buffer[recording_buffer_ptr] = (uint16_t)(int(alti));
        recording_buffer_ptr = (recording_buffer_ptr + 1) % REC_BUFFER_SIZE;
    } else {
        if (!recording_to_flash) {
            recording_to_flash = true;
            // start recording
            Serial.printf("started recording\n");
            //copy circular  buffer to flash
            uint32_t len = REC_BUFFER_SIZE - recording_buffer_ptr;
            write_flight_data((uint8_t *)recording_buffer + (recording_buffer_ptr * 2), len * 2);
            write_flight_data((uint8_t *)recording_buffer, recording_buffer_ptr * 2);
        }
        uint16_t alt = int(alti) & 0xFFFF;
        write_flight_data((uint8_t*) &alt, 2);
    }
}



void erase_video_space() {
    // erase pages 300 to 500
    for (int i = START_MEDIA; i < min(START_MEDIA+100, 500); i++) {
        flash.blockErase32K(i * 32000);
    }
}

uint32_t flash_addr = START_MEDIA * 32000;
void write_video_data(uint16_t data) {
    // write to page 300 onwards
    flash.writeBytes(flash_addr, (uint8_t *)&data, 2);
    while (flash.busy());
    flash_addr += 2;
}

void flash_read_bytes(uint32_t addr, uint8_t *buf, uint32_t len) {
    flash.readBytes(addr, buf, len);
}