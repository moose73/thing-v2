#include "battery_monitor.h"



const unsigned int BATTERY_CAPACITY = 2000; 

void setup_battery(void)
{


  if (!lipo.begin()) 
  {
    Serial.println("Error: Unable to communicate with BQ27441.");
    Serial.println("  Check wiring and try again.");

    while (1) ;
  }
  Serial.println("Connected to BQ27441!");
  
  // Uset lipo.setCapacity(BATTERY_CAPACITY) to set the design capacity
  // of your battery.
  lipo.setCapacity(BATTERY_CAPACITY);
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

uint16_t get_soc()
{
  return lipo.soc();
}

uint16_t get_voltage()
{
  return lipo.voltage();
}

int16_t get_current()
{
  
  return (int16_t) (((float)lipo.current(AVG)) / 1.35);
}

int16_t get_power()
{
  return lipo.power();
}

uint16_t get_capacity_remaining() {
  return lipo.capacity(REMAIN);
}

uint16_t get_capacity_full() {
  return lipo.capacity(FULL);
}


