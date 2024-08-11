#include "battery_monitor.h"


void setupBQ27441(void)
{


  if (!lipo.begin()) 
  {
    Serial.println("Error: Unable to communicate with BQ27441.");
    Serial.println("  Check wiring and try again.");
    Serial.println("  (Battery must be plugged into Battery Babysitter!)");
    while (1) ;
  }
  Serial.println("Connected to BQ27441!");
  
  // Uset lipo.setCapacity(BATTERY_CAPACITY) to set the design capacity
  // of your battery.
  lipo.setCapacity(BATTERY_CAPACITY);
}

float get_soc()
{
  return lipo.soc();
}

float get_voltage()
{
  return lipo.voltage();
}

float get_current()
{
  return lipo.current(AVG);
}

float get_power()
{
  return lipo.power();
}


void printBatteryStats()
{
  // Read battery stats from the BQ27441-G1A
  unsigned int soc = lipo.soc();  // Read state-of-charge (%)
  unsigned int volts = lipo.voltage(); // Read battery voltage (mV)
  int current = lipo.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo.capacity(REMAIN); // Read remaining capacity (mAh)
  int power = lipo.power(); // Read average power draw (mW)
  int health = lipo.soh(); // Read state-of-health (%)

  // Now print out those values:
  String toPrint = String(soc) + "% | ";
  toPrint += String(volts) + " mV | ";
  toPrint += String(current) + " mA | ";
  toPrint += String(capacity) + " / ";
  toPrint += String(fullCapacity) + " mAh | ";
  toPrint += String(power) + " mW | ";
  toPrint += String(health) + "%";
  
  Serial.println(toPrint);
}

uint32_t update_battery_display(TFT_eSPI tft)
{
  int font_size = 5;
  tft.fillRect(0, 0, 320, 30, TFT_WHITE);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(2);

  tft.printf("%d%% | %dmV | %dmA | %d / %d mAh", lipo.soc(), lipo.voltage(), lipo.current(AVG), lipo.capacity(REMAIN), lipo.capacity(FULL));
  Serial.printf("%d%% | %dmV | %dmA | %d / %d mAh\n", lipo.soc(), lipo.voltage(), lipo.current(AVG), lipo.capacity(REMAIN), lipo.capacity(FULL));
  return 1000 * 1000;
}

