# Can_DS18B20
A standalone non-blocking library based on Maxim DS18B20 datasheet. I took great care to make every command and variable self-explanatory and as close to datasheet as possible. Doesn't have Search Rom function since I only had one counterfeit DS18B20 and doubt I can find genuine ones to test it, you can check out authenticity of yours with [this library](https://github.com/cpetrich/counterfeit_DS18B20).

## Creating Object
``` c++
#include "Can_DS18B20.h"

Can_DS18B20 ds18b20(8);
```
This will create a DS18B20 object and fetch scratchpad contents, power mode and ROM code. 
## Using Commands
All commands must follow this sequence or you will encounter problems.
```c++
// Initialization
ds18b20.reset();
// ROM command
ds18b20.matchROM(); // ds18b20.skipROM()
// DS18B20 Function Command
ds18b20.convertT();
```
## Accessing Variables
```c++
Serial.print("ROM code: ");
for (uint8_t printer = 0; printer < 7; printer++) {
  Serial.print(ds18b20._rom[printer] >> 4, HEX);
  Serial.print(ds18b20._rom[printer] & 0x0F, HEX);
  Serial.print("-");
}
Serial.print(ds18b20._rom[7] >> 4, HEX);
Serial.print(ds18b20._rom[7] & 0x0F, HEX);
Serial.println("");

Serial.print("Temperature: ");
Serial.println(ds18b20._temperature);

Serial.print("High temperature alarm: ");
Serial.println(ds18b20._alarm_high);

Serial.print("Low temperature alarm: ");
Serial.println(ds18b20._alarm_low);

Serial.print("Conversion resolution: ");
Serial.println(ds18b20._resolution);

Serial.print("Is detected: ");
Serial.println(ds18b20._is_detected);

Serial.print("Is parasite: ");
Serial.println(ds18b20._is_parasite);

Serial.print("Is alarm detected: ");
Serial.println(ds18b20._is_alarm_detected);
```
## Changing Scratchpad Variables and Writing to EEPROM
This example changes sensors high temperature alarm, low temperature alarm and conversion bit resolution to 100°C, -50°C and 12-bits then copy them to EEPROM.
```c++
ds18b20.reset();
ds18b20.matchROM();
ds18b20.writeScratchpad(100, -50, 12);
ds18b20.reset();
ds18b20.matchROM();
ds18b20.copyScratchpad();
```
Alarm registers are 8 bit and signed as two's complement. Resolution is between 9 to 12 bits and function will default to 12 bits if invalid input is given.

## License
Public Domain, feel free to do anything you want with the code.
