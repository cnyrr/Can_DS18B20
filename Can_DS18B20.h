#ifndef Can_DS18B20_H
#define Can_DS18B20_H

#include "Arduino.h"

class Can_DS18B20 {
  public:
    Can_DS18B20(uint8_t PIN);
    ~Can_DS18B20();
    
    // DS18B20 info.
    uint8_t _rom[8] = {0, 0, 0, 0, 0, 0, 0, 0};                 
    float _temperature;            // First two bytes of the scratchpad.
    int8_t _alarm_high;            // Third byte of the scratchpad.
    int8_t _alarm_low;             // Fourth byte of the scratchpad.
    uint8_t _resolution = 12;      // Fifth byte of the scratchpad.
    uint8_t _is_parasite;
    uint8_t _is_detected;
    uint8_t _is_alarm_detected;

    // ----- OneWire Command Set ----- //

      // Initialization Procedure
      void reset();

      // Write and Read Slots
      void writeZeroSlot();
      void writeOneSlot();
      uint8_t readSlot();

      // Search ROM 0xF0
      // Finds ROM codes of all devices on bus.
      /*
          Not implemented, sadly I only have one "counterfeit" device that
         doesn't even work in parasite mode. You can check out authenticity yours with this
         guys awesome library.
         https://github.com/cpetrich/counterfeit_DS18B20
      */

      // Read ROM 0x33
      void readROM();
      // Gets 64-bit ROM code of DS18B20 if there is only one device on bus.

      // Match ROM 0x55
      void matchROM();
      // Allows the master to address the device with matching ROM.

      // Skip ROM 0xCC
      void skipROM();
      // Allows the master to address all devices on the bus.

      // Alarm Search 0xEC
      void alarmSearch();
      // Checks if there is a device with active alarm.

    // ----- DS18B20 Function Command Set ----- //

      // Convert T 0x44
      void convertT();
      // Initiates temperature conversion.

      // Write Scratchpad 0x4E
      void writeScratchpad(int8_t high_alarm, int8_t low_alarm, uint8_t precision);
      // Allows master to configure alarm and precision registers.
    
      // Read Scratchpad 0xBE
      /* Doesn't check CRC for now. */
      void readScratchpad();
      // Allows master to read the contents of the scratchpad.

      // Copy Scratchpad 0x48
      void copyScratchpad();
      // Copies contents of the alarm and precision registers to EEPROM.

      // Recall EEPROM 0xB8
      void recallEEPROM();
      // Copies EEPROM contents to scratchpad.

      // Read Power Supply 0xB4
      void readPowerSupply();
      // Checks if the DS18B20 is in parasite power mode.

    // ----- Custom Functions ----- //

      // Writes up to 4 bytes into OneWire bus.
      void writeBytes(uint32_t value, uint8_t byte_count);

      // Calculates CRC.
      /* Not implemented yet. */

  private:
    const uint8_t _DATAPIN;
};
#endif