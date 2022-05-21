#ifndef STORE_H
#define STORE_H

void save_to_eeprom(unsigned int addr, int f);
uint16_t read_eeprom(unsigned int addr);

#endif
