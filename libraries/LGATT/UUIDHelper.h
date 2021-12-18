

#ifndef __UUID_HELPER_H__
#define __UUID_HELPER_H__

#include <Arduino.h>

uint32_t uuid_squeeze(uint8_t *str, uint8_t ch);

uint8_t chr2u8(char chr);

void str2uuid_int(const char *str, uint8_t *uuid, uint8_t seperator);

#endif