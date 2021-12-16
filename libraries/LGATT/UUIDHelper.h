

#ifndef __UUID_HELPE_H__
#define __UUID_HELPE_H__

static uint32_t uuid_squeeze(uint8_t *str, uint8_t ch);

static uint8_t chr2u8(char chr);

static void str2uuid_int(const char *str, uint8_t *uuid);

#endif