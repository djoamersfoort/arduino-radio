#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "store.h"

// https://roboticsbackend.com/arduino-store-int-into-eeprom/

void save_to_eeprom(unsigned int addr, int f) {
	Serial.print("Writing '"); Serial.print(f, DEC); Serial.println("'");

	EEPROM.update(addr    , f >> 8);
	EEPROM.update(addr + 1, f & 0xff);
}

uint16_t read_eeprom(unsigned int addr) {
	/*
	byte value[sizeof(uint16_t)];
	for(byte i = 0; i < sizeof(uint16_t); i++) {
		Serial.print("Reading byte '"); Serial.print(value[i], DEC); Serial.println("'");
		value[i] = EEPROM.read(addr + i);
	}
	//return value[0] + 256 * value[1];
	//return int(value);
	return value[0];
	*/

	return (EEPROM.read(addr) << 8) + EEPROM.read(addr + 1);
}
