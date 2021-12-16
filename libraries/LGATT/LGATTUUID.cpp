#include "LGATTUUID.h"
#include "UUIDHelper.h"

LGATTUUID::LGATTUUID()
{
    memset(&_uuid, 0, sizeof(_uuid));
}

LGATTUUID::LGATTUUID(uint16_t uuid)
{
    _uuid.length = sizeof(uint16_t);
    _uuid.uuid[1] = (uint8_t)(uuid >> 8);
    _uuid.uuid[0] = (uint8_t)uuid;
    _uuid.uuid[2] = 0;
}

LGATTUUID::LGATTUUID(const char *uuid)
{
    str2uuid(_uuid.uuid, uuid);
    _uuid.length = sizeof(_uuid.uuid);
    // Serial.print(*this);
    // Serial.println();
}

LGATTUUID::LGATTUUID(const vm_bt_uuid_with_length_t &uuid)
{
    _uuid = uuid;
}

bool LGATTUUID::operator==(const LGATTUUID &uuid) const
{
    if (_uuid.length != uuid._uuid.length)
        return false;
    return !memcmp(_uuid.uuid, uuid._uuid.uuid, _uuid.length);
}

bool LGATTUUID::operator==(const char *uuid) const
{
    LGATTUUID tmp_uuid(uuid);
    if (tmp_uuid == *this)
    {
        return true;
    }

    return false;
}

// Overloaded index operator to allow getting and setting individual octets of the address
uint8_t LGATTUUID::operator[](int index) const
{
    return _uuid.uuid[index];
}

uint8_t &LGATTUUID::operator[](int index)
{
    return _uuid.uuid[index];
}

// Overloaded copy operators to allow initialisation of LGATTUUID objects from other types
LGATTUUID &LGATTUUID::operator=(const char *uuid)
{
    _uuid.length = sizeof(_uuid.uuid);
    str2uuid(_uuid.uuid, uuid);

    return *this;
}

LGATTUUID &LGATTUUID::operator=(uint16_t uuid)
{
    _uuid.length = sizeof(uint16_t);
    _uuid.uuid[1] = (uint8_t)(uuid >> 8);
    _uuid.uuid[0] = (uint8_t)uuid;
    _uuid.uuid[2] = 0;
    return *this;
}

LGATTUUID &LGATTUUID::operator=(const vm_bt_uuid_with_length_t &uuid)
{
    _uuid = uuid;
    return *this;
}

size_t LGATTUUID::printTo(Print &p) const
{
    size_t n = 0;
    for (int i = _uuid.length - 1; i >= 0; i--)
    {
        if (_uuid.uuid[i] <= 0xf)
            p.print('0');
        n += p.print(_uuid.uuid[i], HEX);
        if (12 == i ||
            10 == i ||
            8 == i ||
            6 == i)
            n += p.print('-');
    }

    return n;

    return 0;
}

void LGATTUUID::str2uuid(uint8_t *uuid_dst, const char *uuid) const
{
    str2uuid_int(uuid, uuid_dst, '-');
}
