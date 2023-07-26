#include "Can_DS18B20.h"

Can_DS18B20::Can_DS18B20(uint8_t PIN): _DATAPIN(PIN) {
  // Enter Tx mode.
  digitalWrite(_DATAPIN, HIGH);
  pinMode(_DATAPIN, OUTPUT);

  reset();
  skipROM();
  readPowerSupply();
  reset();
  skipROM();
  readScratchpad();
  reset();
  readROM();
}

Can_DS18B20::~Can_DS18B20() {}

// ----- OneWire Command Set ----- //

void Can_DS18B20::reset() {
  // Start of the time critical code.
  noInterrupts();

  // First part of the reset signal is at least 480 microseconds of pull down.
  digitalWrite(_DATAPIN, LOW);
  delayMicroseconds(550);

  // Release the channel to enter Rx mode.
  pinMode(_DATAPIN, INPUT);

  // Channel resistor will pull up.
  // When DS18B20 detects the rising voltage, it will wait 15-60 microseconds.
  // Then it pull down for 60-240 microseconds. 
  delayMicroseconds(70);

  // Channel will be LOW if DS18B20 responds.
  _is_detected = !digitalRead(_DATAPIN);

  // Second part of reset signal is at least 480 microseconds of wait.
  delayMicroseconds(550);

  // Check if the resistor pulled up.
  if (_is_detected) {_is_detected = digitalRead(_DATAPIN);}

  // Enter Tx mode.
  digitalWrite(_DATAPIN, HIGH);
  pinMode(_DATAPIN, OUTPUT);

  // End of the time critical code.
  interrupts();
}

void Can_DS18B20::writeZeroSlot() {
  // To write zero we keep the bus LOW from at least 60 microseconds to end of time slot.
  // Warning: Keeping the bus LOW too long will cause reset.
  digitalWrite(_DATAPIN, LOW);
  delayMicroseconds(65);
  digitalWrite(_DATAPIN, HIGH);

  // Channel needs to be free for at least 1 microseconds following the time slot.
  delayMicroseconds(3);
}

void Can_DS18B20::writeOneSlot() {
  // To write one we pull the bus down then release within 15 microseconds.
  digitalWrite(_DATAPIN, LOW);
  delayMicroseconds(3);

  // Enter Tx mode.
  // We need to go HIGH within 10 microseconds for parasite powered DS18B20 to make some commands work.
  digitalWrite(_DATAPIN, HIGH);
  pinMode(_DATAPIN, OUTPUT);

  // Minimum write slot duration is 60 microseconds.
  delayMicroseconds(65);
}

uint8_t Can_DS18B20::readSlot() {
  // Bit to return.
  uint8_t received_bit = 0;

  // To initate a read time slot we pull LOW for at least 1 microsecond, then release.
  // Note: It is advised to keep pull duration minimum.
  digitalWrite(_DATAPIN, LOW);
  delayMicroseconds(3);
  pinMode(_DATAPIN, INPUT);

  // DS18B20 transmits a "1" by leaving the bus HIGH.
  // DS18B20 transmits a "0" by pulling the bus LOW.
  // Transmitted data is only valid for 15 microseconds following our pull.
  // Note: It is advised to sample towards the end of our 15 microsecond window.
  delayMicroseconds(10);
  received_bit = digitalRead(_DATAPIN);

  // A read slot must be at least 60 microseconds.
  delayMicroseconds(80);

  // Enter Tx mode.
  digitalWrite(_DATAPIN, HIGH);
  pinMode(_DATAPIN, OUTPUT);

  // Recovery period must be at least 1 microseconds.
  delayMicroseconds(3);

  return received_bit;
}

void Can_DS18B20::readROM() {
  writeBytes(0x33, 1);

  for (uint8_t bit_position = 0; bit_position < 72; bit_position++) {
    // Bitwise OR every bit to their place.
    _rom[bit_position/8] |= (readSlot() << (bit_position % 8));
  }
}

void Can_DS18B20::matchROM() {
  writeBytes(0x55, 1);

  for (uint8_t counter = 0; counter < 8; counter++) {
    writeBytes(_rom[counter], 1);
  }
}

void Can_DS18B20::skipROM() {
  writeBytes(0xCC, 1);
}

void Can_DS18B20::alarmSearch() {
  writeBytes(0xEC, 1);

  _is_alarm_detected = readSlot();
}

// ----- DS18B20 Function Command Set ----- //

void Can_DS18B20::convertT() {
  writeBytes(0x44, 1);

  // We can't poll the sensor if its parasite powered. 
  if (_is_parasite) {
    uint32_t poll_time = millis();
    while (poll_time - millis() < 800 / pow(2, 12 - _resolution)) {;}
  }
  else {
    while(readSlot() == 0){;} 
  }
}

void Can_DS18B20::writeScratchpad(int8_t _new_high_alarm, int8_t _new_low_alarm, uint8_t _new_resolution) {
  writeBytes(0x4E, 1);

  // We should prepare our data to pass into writeBytes().
  uint32_t constructed_byte = 0;

  // Resolution range is 12 to 9 bits.
  if (_new_resolution > 12 || _new_resolution < 9) {_new_resolution = 12;}
  else {_new_resolution -= 9;}

  // Third byte is configuration register.
  constructed_byte = ((uint32_t)((_new_resolution << 5) | 0b00011111)) << 16;

  // Second byte is low temperature alarm register.
  constructed_byte |= (((int32_t) (_new_low_alarm)) << 8) & 0xFF00;

  // First byte is high temperature alarm register.
  constructed_byte |= ((int32_t) (_new_high_alarm)) & 0xFF;

  writeBytes(constructed_byte, 3);
}

void Can_DS18B20::readScratchpad() {
  // Temporarily holds scratchpad data.
  int8_t scratchpad[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

  writeBytes(0xBE, 1);
  
  for (uint8_t bit_position = 0; bit_position < 72; bit_position++) {
    // Bitwise OR every bit to their place.
    scratchpad[bit_position/8] |= (readSlot() << (bit_position % 8));
  }

  /* TO DO: Implement CRC. */

  _resolution = (scratchpad[4] >> 5) + 9;

  _alarm_low = scratchpad[3];

  _alarm_high = scratchpad[2];

  _temperature = ((float) ((((int16_t) scratchpad[1]) << 8) | (scratchpad[0] & 0xFF)) / 16);
}

void Can_DS18B20::copyScratchpad() {
  writeBytes(0x48, 1);

  // DS18B20 needs to charge its parasite capacitor for at least 10ms.
  if(_is_parasite) {
    uint32_t poll_time = millis();
    while (poll_time - millis() < 12) {;}
  }
}

void Can_DS18B20::recallEEPROM() {
  writeBytes(0xB8, 1);

  // Block unless transfer is complete.
  while (readSlot() == 0) {;}
}

void Can_DS18B20::readPowerSupply() {
  writeBytes(0xB4, 1);

  // Parasite powered DS18B20 will pull down.
  _is_parasite = !readSlot();
}

void Can_DS18B20::writeBytes(uint32_t value, uint8_t byte_count) {
  // How many bits to send.
  byte_count = byte_count * 8;

  // Start of the time critical code.
  noInterrupts();

  for (uint8_t bit_count = 0; bit_count < byte_count; bit_count++) {
    if (value & 0x1) {
      writeOneSlot();
    }
    else {
      writeZeroSlot();
    }
    value = value >> 1;
  }
  
  // End of the the time critical code.
  interrupts();
}
