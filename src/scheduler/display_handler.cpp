#include "display_handler.h"

TFT_eSPI tft = TFT_eSPI();
FT6236 ts = FT6236();

bool touched = false;
void IRAM_ATTR ts_trigger() { touched = true; }

void setup_power_off_touch (bool shift=false, bool trig=false);
uint32_t handle_touch();
void check_power_off_touch(int x, int y);
void start_poweroff_sequence();
int draw_yana_image(int flash_start, int upsample = 1);

void draw_bitmap(char* bmp, int x, int y, int w, int h, uint16_t color) {
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      if (bmp[i + j * w] == 1) {
        tft.drawPixel(x + i, y + j, color);
      }
    }
  }
}

void setup_display() {
  tft.init();
  delay(2000);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);

  ts.begin(20, -1, -1);
  pinMode(LCD_CTP_IRQ, INPUT);
  attachInterrupt(LCD_CTP_IRQ, ts_trigger, FALLING);
  setup_power_off_touch(false, false);

  add_task(handle_touch, 0);
}

uint32_t handle_touch(void) {
  if (touched) {
    touched = false;
    TS_Point p = ts.getPoint();
    if (!(p.x == 0 && p.y == 0)) {
        check_power_off_touch(p.x, p.y);
      }
  }
  return 50 * 1000;
}

void displayFSMessage(String str, bool background=true, int delay_time=2000) {
  if (background) {
    tft.fillScreen(TFT_BLUE);
  }
  // tft.fillScreen(TFT_BLUE);
  tft.setTextSize(4);
  tft.setCursor(150, 20);
  tft.setTextColor(TFT_RED);
  tft.printf("%s", str); 
  delay(delay_time);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
}
void setup_power_off_touch (bool shift, bool trig) {
  int ofs = 0;
  if (shift) {
    ofs = 40;
  }
  tft.fillRect(0, 200, 80, 40, TFT_WHITE);
  if (!trig) {
    tft.fillRect(ofs, 200, 40, 40, TFT_LIGHTBLUE);
  } else {
    tft.fillRect(ofs, 200, 40, 40, TFT_PINK); 
  }
  tft.setCursor(10 + ofs, 210);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.printf("OFF"); 
}

uint8_t power_off_seq_state = 0;
uint32_t power_off_seq_last_touch = 0;
int8_t power_off_watchdog_task = -1;

uint32_t reset_poweroff_seq() {
  power_off_seq_state = 0;
  setup_power_off_touch(false, false);
  return 0;
}

void check_power_off_touch(int x, int y) {
  if (micros() - power_off_seq_last_touch < 500 * 1000) {
    return;
  } 
  power_off_seq_last_touch = micros();
  
  if (power_off_seq_state == 4) {
    Serial.println("Powering off");
    power_off_seq_state = 0;
    setup_power_off_touch(false);

    #ifdef YANA
    Serial.printf("starting yana image draw\n");
    draw_yana_image(300 * 32000, 2);
    displayFSMessage("bye :(", false, 5000);
    #endif
    // displayFSMessage("bye :(");

    start_poweroff_sequence();
    return;
  }

  if (power_off_seq_state % 2 == 0) {
    if (x >= 0 && x <= 40 && y >= 200 && y <= 240) {
      if (power_off_watchdog_task != -1) {
        remove_task(power_off_watchdog_task);
      }
      power_off_watchdog_task = add_task(reset_poweroff_seq, 1e6);
      power_off_seq_state++;
      setup_power_off_touch(true, true);
    } else {
      power_off_seq_state = 0;
      setup_power_off_touch(false, false);
    }
  } else {
    if (x >= 40 && x <= 80 && y >= 200 && y <= 240) {
      if (power_off_watchdog_task != -1) {
        remove_task(power_off_watchdog_task);
      }
      power_off_watchdog_task = add_task(reset_poweroff_seq, 1e6);
      power_off_seq_state++;
      setup_power_off_touch(false, true);
    } else {
      power_off_seq_state = 0;
      setup_power_off_touch(false, false);
    }
  } 
}

uint32_t update_battery_stats() {
  int font_size = 5;
  tft.fillRect(0, 0, 320, 30, TFT_WHITE);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.printf("%d%% | %dmV | %dmA", get_soc(), get_voltage(),
             get_current());
  return 1000 * 1000;
}


String prev_alti_string = "";
String prev_vspeed_string = "";
uint32_t display_refresh_rate = 80 * 1000; 

uint32_t update_altimeter_display() {
  tft.drawFastHLine(0, 40, 320, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);
  
  uint32_t rec_time = micros();
  float alti = get_display_alti();

  String altiString;
  if (alti > 1000 && alti < 10000) {
    alti = round(alti/10) * 10;
    altiString = String(alti / 1000);
    altiString.concat(String('k'));
  } else if (alti > 10000) {
    alti = round(alti/100) * 100;
    altiString = String(alti / 1000).substring(0, 4);
    altiString.concat(String('k'));
  } else {
    altiString = String(int(round(alti / 10))*10);
  }

  float vspeed = get_vspeed_1();
  vspeed = round((vspeed / 1.467) / 10) * 10;
  String vspeedString = String(int(vspeed));

  if (altiString.equals(prev_alti_string) && vspeedString.equals(prev_vspeed_string)) {
    return display_refresh_rate;
  }
  prev_alti_string = altiString;
  prev_vspeed_string = vspeedString;

  State current_state = get_current_state();

  if (current_state == FLIGHT && alti > 4000) {
    tft.fillRect(0, 41, 320, 78, TFT_GREENYELLOW);
  } else if (current_state == FREEFALL) {
    if (alti > 6000) {
     tft.fillRect(0, 41, 320, 78, TFT_GREEN); 
    } else if (alti > 4000) {
      tft.fillRect(0, 41, 320, 78, TFT_YELLOW);
    } else {
      tft.fillRect(0, 41, 320, 78, TFT_RED);
    }
  } else {
    tft.fillRect(0, 41, 320, 78, TFT_WHITE);
  }

  // tft.fillRect(0, 41, 320, 78, TFT_WHITE);


  float width = (tft.drawString(altiString.substring(0, 6), 160, 50, 4));
  tft.drawFastHLine(0, 120, 320, TFT_BLACK);
  tft.setTextSize(1);

  tft.drawString(" ft", 180 + width/2, 85, 4); //4 is best sofar


  Serial.printf("State is %d\n", current_state);
  //show the PLANE_BMP bitmap on the right edge if in flight mode
  if (current_state == FLIGHT) {
   draw_bitmap((char*)plane_bmp, 290, 95, 20, 20, TFT_BLUE); 
  } else if (current_state == FREEFALL) {
    draw_bitmap((char*)freefall_bmp, 290, 85, 30, 30, TFT_BLUE);
  } else if (current_state == CANOPY) {
    draw_bitmap((char*)canopy_bmp, 290, 85, 30, 30, TFT_BLUE);
  }  else {
    Serial.printf("displaying nothing ???\n");
  }

  tft.fillRect(0, 121, 160, 79, TFT_WHITE);
  tft.setCursor(10, 130);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);

  tft.drawFastVLine(160, 121, 79, TFT_BLACK);
  tft.drawFastHLine(0, 199, 320, TFT_BLACK);

  //format vspeed string. Convert to mph, round to nearest 10

  width = tft.drawString(vspeedString.substring(0, 4), 80, 130, 4);

  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(" mph", 28, 130, 2);
  tft.drawLine(5, 125, 5, 136, TFT_BLACK);

  tft.drawLine(5, 125, 8, 128, TFT_BLACK);
  tft.drawLine(5, 125, 2, 128, TFT_BLACK);
  tft.drawLine(5, 136, 8, 133, TFT_BLACK);
  tft.drawLine(5, 136, 2, 133, TFT_BLACK);

  draw_bitmap((char*)speedo_bmp, 10, 121, 20, 20, TFT_BLACK);
  return display_refresh_rate;
}

uint8_t ctr = 0;

uint32_t sleep_loop() {

  TS_Point p = ts.getPoint();
  if (p.x == 0 && p.y == 0) {
    esp_sleep_enable_timer_wakeup(2 * 1000 * 1000);
    esp_light_sleep_start();
    return 1;
  } else {
    ctr += 1;
    if (ctr > 30) {
      digitalWrite(LCD_BL_EN, HIGH);
      #ifdef YANA
      draw_yana_image(400 * 32000);
      displayFSMessage("Hi!!", false, 5000);
      #else
      displayFSMessage("Hi!!");
      #endif
      ESP.restart();
    }
    return 100 * 1000;
  }
} 

void start_poweroff_sequence() {
  clear_all_tasks();
  digitalWrite(LCD_BL_EN, LOW); 
  add_task(sleep_loop, 0);
}

uint32_t update_wifi_rssi_bars() {
  //draw the rssi arcs on the top right of the screen
  tft.drawRect(220, 0, 20, 20, TFT_WHITE);
  // uint8_t rssi = get_rssi_bars();
  // Serial.printf("RSSI is %d\n", rssi);

  return 3 * 1e6;
}

void draw_vline_bitmap(uint16_t* bmp, int w, int y, int upsample) {
  for (int y_d = y; y_d < y + upsample; y_d++) {
    for (int i = 0; i < w; i++) {
      for (int x_d = i * upsample; x_d < (i + 1) * upsample; x_d++) {
        tft.drawPixel(x_d, y_d, bmp[i]);
      }
    }
  }
}

int draw_yana_image(int flash_start, int upsample) {
  uint16_t buf[int(320 / upsample)];

  for (int y = 0; y < 240; y+=upsample) {
    flash_read_bytes(flash_start, (uint8_t*)buf, int((2 * 320) / upsample));
    draw_vline_bitmap(buf, int(320 / upsample), y, upsample);
    flash_start += ((2*320) / upsample);

  }
  return flash_start;

  

}