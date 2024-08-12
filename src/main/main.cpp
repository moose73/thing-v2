#include "battery_monitor/battery_monitor.h"
#include "pins.h"
#include <Arduino.h>
#include <FT6236.h>
#include <LPS22HHSensor.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Filters.h>
#include "utils/utils.h"
#include "flash_record/flash_record.h"

uint32_t update_battery_display_wrapper(void);
uint32_t handle_touch(void);
void power_off(void);
uint32_t reset_poweroff_seq(void);
uint32_t reset_zero_seq(void);
uint32_t display_alti_1(void);
uint32_t display_alti_2(void);
void zero_altis();

struct Task {
  uint32_t (*taskCall)(void);
  uint32_t nexttime;
  bool enabled;
};

Task taskTable[] = {
  {update_battery_display_wrapper, 0, true},
  {handle_touch, 0, true},
  {reset_poweroff_seq, 0, false}, //keep this as pos 2, is hardcoded!
  {reset_zero_seq, 0, false}, //keep this as pos 3, is hardcoded!
  {display_alti_1, 0, true},
  {display_alti_2, 0, true}
};

TFT_eSPI tft = TFT_eSPI();
FT6236 ts = FT6236();
float alti_1_offs = 0;
float alti_2_offs = 0;

bool touched = false;
void IRAM_ATTR ts_trigger() { touched = true; }

uint32_t update_battery_display_wrapper(void) {
  return update_battery_display(tft);
}


void setup_power_off_touch (bool shift = false, bool trig  = false) {
  int ofs = 0;
  if (shift) {
    ofs = 40;
  }
  tft.fillRect(0, 200, 80, 40, TFT_WHITE);
  if (!trig) {
    tft.fillRect(ofs, 200, 40, 40, TFT_LIGHTGREY);
  } else {
    tft.fillRect(ofs, 200, 40, 40, TFT_PINK); 
  }
  tft.setCursor(10 + ofs, 220);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.printf("OFF"); 
}

void setup_zero_touch(bool shift = false, bool trig = false) {

  int ofs = 0;
  if (shift) {
    ofs = -40;
  }
  tft.fillRect(240, 200, 80, 40, TFT_WHITE);
  if (!trig) {
    tft.fillRect(ofs + 280, 200, 40, 40, TFT_LIGHTGREY);
  } else {
    tft.fillRect(ofs + 280, 200, 40, 40, TFT_PINK); 
  }
  tft.setCursor(290 + ofs, 220);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.printf("ZER"); 
}

int power_off_seq_state = 0;
uint32_t power_off_seq_last_touch = 0;

void displayFSMessage(String str) {
  tft.fillScreen(TFT_BLUE);
  tft.setCursor(30, 120);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(10);
  tft.printf("%s", str); 
  delay(2000);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
}

void power_off() {
  Serial.printf("entering power off\n");
  alti_1_offs = 0;
  digitalWrite(LCD_BL_EN, LOW);
  int ctr = 0;

  while (true) {
    // touched = !digitalRead(LCD_CTP_IRQ);

    while (touched) {
      TS_Point t = ts.getPoint();
      // Serial.printf("x is %d, y is %d, touched is %d\n", t.x, t.y, touched);
      if (t.x !=0 && t.y != 0) {
          if (ctr == 60) {
            digitalWrite(LCD_BL_EN, HIGH);
            displayFSMessage("Hi!!");
            ESP.restart();
            return;
          }

          ctr += 1;
          Serial.printf("ctr is %d\n", ctr);
          touched = 0;
          delay(50);
      } else {
        touched = 0;
      }
    } 
    esp_sleep_enable_timer_wakeup(2 * 1000000);
    esp_light_sleep_start();
    ctr = 0;
    touched = digitalRead(LCD_CTP_IRQ) ? false : true; 
    Serial.printf("touched is %d\n", touched);  
    delay(10);
  }
}

uint32_t reset_poweroff_seq() {
  power_off_seq_state = 0;
  setup_power_off_touch(false, false);
  return 0;
}


int zero_seq_state = 0;
int zero_state_last_touch = 0;

uint32_t reset_zero_seq() {
  zero_seq_state = 0;
  setup_zero_touch(false, false);
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
    displayFSMessage("bye :(");
    power_off();
    return;
  }

  if (power_off_seq_state % 2 == 0) {
    if (x >= 0 && x <= 40 && y >= 200 && y <= 240) {
      power_off_seq_state++;
      taskTable[2].enabled = true;
      taskTable[2].nexttime = micros() + 1e6;
      setup_power_off_touch(true, true);
    } else {
      power_off_seq_state = 0;
      setup_power_off_touch(false, false);
    }
  } else {
    if (x >= 40 && x <= 80 && y >= 200 && y <= 240) {
      power_off_seq_state++;
      taskTable[2].enabled = true;
      taskTable[2].nexttime = micros() + 1e6;
      setup_power_off_touch(false, true);
    } else {
      power_off_seq_state = 0;
      setup_power_off_touch(false, false);
    }
  } 
}

void check_zero_touch(int x, int y) {

  if (micros() - zero_state_last_touch < 500 * 1000) {
    return;
  } 
  zero_state_last_touch = micros();
  // Serial.printf("ZOT: %d, xy: %d %d\n", zero_seq_state, x, y);

  if (zero_seq_state == 4) {
    Serial.println("Zeroing");
    zero_seq_state = 0;
    setup_zero_touch(false);
    zero_altis();
    return;
  }

  if (zero_seq_state % 2 == 0) {
    if (x >= 280 && x <= 320 && y >= 200 && y <= 240) {
      zero_seq_state++;
      taskTable[3].enabled = true;
      taskTable[3].nexttime = micros() + 1e6;
      setup_zero_touch(true, true);
    } else {
      zero_seq_state = 0;
      setup_zero_touch(false, false);
    }
  } else {
    if (x >= 240 && x <= 280 && y >= 200 && y <= 240) {
      zero_seq_state++;
      taskTable[3].enabled = true;
      taskTable[3].nexttime = micros() + 1e6;
      setup_zero_touch(false, true);
    } else {
      zero_seq_state = 0;
      setup_zero_touch(false, false);
    }
  } }

uint32_t handle_touch(void) {
  if (touched) {
    touched = false;
    TS_Point p = ts.getPoint();
    if (!(p.x == 0 && p.y == 0)) {
        check_power_off_touch(p.x, p.y);
        check_zero_touch(p.x, p.y);
      }
  }
  return 50 * 1000;
}

#define TASK_COUNT (sizeof(taskTable) / sizeof(struct Task))

SPIClass spi(HSPI);
LPS22HHSensor sensor(&spi, SPI_LPCS);

float pressure_to_altitude(float pressure) {
  return 44330 * (1.0 - pow(pressure / 1013.25, 0.1903)) * 3.28;
}



uint32_t display_alti_1() {
  // Serial.printf("alti loop 1\n");
  tft.drawFastHLine(0, 40, 320, TFT_BLACK);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);
  

  float pressure;
  sensor.GetPressure(&pressure);
  uint32_t rec_time = micros();
  float alti1 = pressure_to_altitude(pressure) - alti_1_offs;
  alti1 = simulate_flight_alti() + alti1;
  // Serial.printf("alti1 was %f, now is %f\n", alti1, alti1 + simulate_flight_alti());

  float alti1_unrounded = alti1;
  tft.fillRect(0, 41, 320, 78, TFT_WHITE);

  // alti1 = millis()/10;
  String altiString;
  if (alti1 > 1000 && alti1 < 10000) {
    alti1 = round(alti1/10) * 10;
    altiString = String(alti1 / 1000);
    altiString.concat(String('k'));
  } else if (alti1 > 10000) {
    alti1 = round(alti1/100) * 100;
    altiString = String(alti1 / 1000).substring(0, 4);
    altiString.concat(String('k'));
  } else {
    
    altiString = String(int(round(alti1 / 10))*10);
    // Serial.println(altiString);
  }

  tft.drawString(altiString.substring(0, 6), 160, 50, 4);

  if (process_gross_singlerec(alti1_unrounded, rec_time)) {
    // Serial.printf("recording , alti is  %f\n", alti1_unrounded);
    tft.fillCircle(300, 60, 5, TFT_RED);
  }

  return 50 * 1000;
}

void zero_altis() {
 float pressure;
float avgbf = 0;
  for (int i = 0; i < 50; i++) {
   sensor.GetPressure(&pressure);
  //  Serial.println(pressure);
    delay(10);
    avgbf += pressure_to_altitude(pressure);
  }

  alti_1_offs = avgbf / 50; 

  Serial.println(alti_1_offs);
}

uint32_t display_alti_2() {
  // Serial.printf("alti loop 2\n");
  tft.drawFastHLine(0, 120, 320, TFT_BLACK);

  // tft.setTextDatum(TC_DATUM);
  // tft.setCursor(160, 170);
  // tft.setTextColor(TFT_BLACK);
  // tft.setTextSize(8);
  // tft.printf("%s", millis() / 10); 


  return 50 * 1000;
}

void setup(void) {
  Serial.begin(115200);

  tft.init();
  delay(2000);
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);

  ts.begin(20, 1, 2);
  pinMode(LCD_CTP_IRQ, INPUT);

  setupBQ27441();

  spi.begin(SPI_CLK, SPI_IO1, SPI_IO0);
  sensor.begin();
  sensor.Enable();
  sensor.SetOutputDataRate(20);

  attachInterrupt(LCD_CTP_IRQ, ts_trigger, FALLING);

  setup_power_off_touch(false);
  setup_zero_touch(false);

  zero_altis();

  init_flash();
}

void loop() {
  for (uint32_t i = 0; i < TASK_COUNT;
       i++) { // for each task, execute if next time >= current time
    uint32_t ticks = micros(); // current time in microseconds
    // Serial.printf("%d, %d, %d\n",ticks, taskTable[i].nexttime,
    // taskTable[i].enabled);
    if (taskTable[i].nexttime - ticks > UINT32_MAX / 2 &&
        taskTable[i].enabled) {
      int retval = taskTable[i].taskCall();
      if (retval > 0) {
        taskTable[i].nexttime = ticks + retval;
      } else {
        taskTable[i].enabled = false;
      }
    }
  }
}