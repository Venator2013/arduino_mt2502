/*
  Copyright (c) 2014 MediaTek Inc.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU Lesser General Public License for more details.
*/

#include "Arduino.h"
#include "storage_driver.h"
#include "LStorage.h"

/*****************************************************************************
 *
 * LFile class
 *
 *****************************************************************************/

LFile::LFile()
{
    _fd = 0;
    _isDir = false;
    _name[0] = 0;
    _drv = 0;
    _bufPos = 0;
}

LFile::LFile(unsigned int fd, boolean isdir, char drv, const char *name)
{
    _fd = fd;
    _isDir = isdir;
    _drv = drv;
    strncpy(_name, name, VM_FS_MAX_PATH_LENGTH);
    _bufPos = 0;
}

LFile::~LFile()
{
    close();
}

size_t LFile::write(uint8_t v)
{
    if (!_fd || _isDir)
        return 0;

    _buf[_bufPos++] = v;
    if (_bufPos == LS_WRITE_BUF_SIZE)
        flush();
    return 1;
}

int LFile::read()
{
    uint8_t buf[1];
    int result;

    if (!_fd || _isDir)
        return -1;

    result = _read(buf, 1, false);
    if (result < 0)
        return result;

    return buf[0];
}

int LFile::peek()
{
    uint8_t buf[1];
    int result;

    if (!_fd || _isDir)
        return -1;

    result = _read(buf, 1, true);
    if (result < 0)
        return result;

    return buf[0];
}

int LFile::available()
{
    linkit_file_general_struct data;

    if (!_fd || _isDir)
        return -1;

    data.fd = _fd;

    LTask.remoteCall(linkit_file_available_handler, &data);

    return data.result;
}

void LFile::flush()
{
    linkit_file_flush_struct data;

    if (!_fd || _isDir || _bufPos == 0)
        return;

    data.fd = _fd;
    data.buf = _buf;
    data.nbyte = _bufPos;

    LTask.remoteCall(linkit_file_flush_handler, &data);

    _bufPos = 0;
}

int LFile::read(void *buf, uint16_t nbyte)
{
    return _read(buf, nbyte, false);
}

int LFile::_read(void *buf, uint16_t nbyte, boolean peek_mode)
{
    linkit_file_read_struct data;

    if (!_fd || _isDir)
        return -1;

    data.fd = _fd;
    data.buf = buf;
    data.nbyte = nbyte;
    data.peek_mode = peek_mode;

    LTask.remoteCall(linkit_file_read_handler, &data);

    return data.result;
}

boolean LFile::seek(uint32_t pos)
{
    linkit_file_seek_struct data;

    if (!_fd || _isDir)
        return false;

    data.fd = _fd;
    data.pos = pos;

    LTask.remoteCall(linkit_file_seek_handler, &data);

    return data.result;
}

uint32_t LFile::position()
{
    linkit_file_general_struct data;

    if (!_fd || _isDir)
        return 0;

    data.fd = _fd;

    LTask.remoteCall(linkit_file_position_handler, &data);

    return data.value;
}

uint32_t LFile::size()
{
    linkit_file_general_struct data;

    if (!_fd || _isDir)
        return 0;

    data.fd = _fd;

    LTask.remoteCall(linkit_file_size_handler, &data);

    return data.value;
}

void LFile::close()
{
    linkit_file_general_struct data;

    if (!_fd)
        return;

    flush();

    data.fd = _fd;

    if (_isDir)
        LTask.remoteCall(linkit_file_find_close_handler, &data);
    else
        LTask.remoteCall(linkit_file_close_handler, &data);

    _fd = 0;
}

LFile::operator bool()
{
    return (_fd || _isDir) ? true : false;
}

LFile &LFile::operator=(const LFile &other)
{
    memcpy(this, &other, sizeof(LFile));
    if (_fd)
        REF(_fd)
    ++;

    return *this;
}

char *LFile::name()
{
    int i, len = strlen(_name);
    if (len == 1)
        return _name;

    for (i = len - 2; i--; i >= 0)
        if (_name[i] == '/')
            break;

    i++;
    return _name + i;
}

boolean LFile::isDirectory(void)
{
    if (_isDir)
        return true;

    return false;
}

LFile LFile::openNextFile(uint8_t mode)
{
    linkit_file_find_struct data = {0};
    if (!_isDir)
        return LFile();

    data.mode = mode;
    data.drv = _drv;
    data.findpath = _name;
    data.findhdl = _fd;

    LTask.remoteCall(linkit_file_find_handler, &data);

    _fd = data.findhdl;

    if (data.result < 0)
    {
        return LFile();
    }

    return LFile(data.fd, data.is_dir, _drv, data.name);
}

void LFile::rewindDirectory(void)
{
    linkit_file_general_struct data;
    if (!_isDir || !_fd)
        return;

    data.fd = _fd;
    LTask.remoteCall(linkit_file_find_close_handler, &data);
    _fd = 0;
}

/*****************************************************************************
 *
 * LDrive class
 *
 *****************************************************************************/

boolean LDrive::general_op(int op, char *filepath)
{
    linkit_drv_general_op_struct data;

    data.filepath = filepath;
    data.op = op;
    data.drv = _drv;

    LTask.remoteCall(linkit_drv_general_handler, &data);

    return data.result;
}

LFile LDrive::open(const char *filename, uint8_t mode)
{
    linkit_drv_open_struct data;

    data.filepath = filename;
    data.mode = mode;
    data.drv = _drv;

    LTask.remoteCall(linkit_drv_read_handler, &data);

    if (!data.result) // fail, return empty object
    {
        LSLOG("open() fail");
        return LFile();
    }

    return LFile(data.fd, data.is_dir, _drv, filename);
}
