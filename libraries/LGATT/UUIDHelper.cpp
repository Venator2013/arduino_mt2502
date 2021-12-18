#include "UUIDHelper.h"

#define UUID_STR_LEN_128 (32)
#define UUID_STR_LEN_16 (8)
#define UUID_STR_LEN_6 (12)
#define UUID_STR_LEN_2 (4)

uint32_t uuid_squeeze(uint8_t *str, uint8_t ch)
{
    uint32_t i, j;
    for (i = 0, j = 0; str[i] != '\0'; i++)
    {
        if (str[i] != ch)
        {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';

    return j;
}

uint8_t chr2u8(char chr)
{
    uint8_t ret = 0;
    if (chr >= '0' && chr <= '9')
    {
        ret = chr - '0';
    }
    else if (chr >= 'a' && chr <= 'f')
    {
        ret = chr - 'a' + 10;
    }
    else if (chr >= 'A' && chr <= 'F')
    {
        ret = chr - 'A' + 10;
    }
    return ret;
}

void str2uuid_int(const char *str, uint8_t *uuid, uint8_t seperator)
{
    uint32_t len = 0;
    uint8_t *uuid_str = NULL;
    uint32_t uuid_len = 0;
    int i, j;
    uint8_t H4 = 0;
    uint8_t L4 = 0;

    len = strlen((const char *)str); // mmi_chset_utf8_strlen(str);
    uuid_str = (uint8_t *)malloc(len + 1);
    if (uuid_str != NULL)
    {
        memset(uuid_str, 0x0, len + 1);
        memcpy(uuid_str, str, len);
        uuid_str[len] = '\0';

        uuid_len = uuid_squeeze((uint8_t *)uuid_str, seperator);

        switch (uuid_len)
        {
        case UUID_STR_LEN_128:
        case UUID_STR_LEN_16:
        case UUID_STR_LEN_6:
        case UUID_STR_LEN_2:
        {
            for (i = 0, j = uuid_len - 2; j >= 0; j -= 2)
            {
                H4 = chr2u8(uuid_str[j]);
                L4 = chr2u8(uuid_str[j + 1]);
                uuid[i] = H4 * 16 + L4;
                i++;
            }
            // uuid->len = i;
        }
        break;
        default:
            break;
        }

        free(uuid_str);
        uuid_str = NULL;
    }
    else
    {
        // APP_LOG("int_gatt_str_to_uuid no memory");
    }
}
