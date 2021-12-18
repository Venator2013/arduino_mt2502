

#include "Arduino.h"

#include "vmchset.h"
#include "vmstdlib.h"
#include "storage_driver.h"

/*****************************************************************************
 *
 * Utility
 *
 *****************************************************************************/

#ifdef LINKITSTORAGE_DEBUG
static void _printwstr(VMWCHAR *filepath_buf)
{
    int i = 0;
    while (filepath_buf[i])
    {
        Serial.write(filepath_buf[i]);
        i++;
    }
    Serial.println();
}
#endif

static boolean _conv_path(char drv, VMCSTR filepath, VMWSTR filepath_buf)
{
    int i = 0;

    memset(filepath_buf, 0, (VM_FS_MAX_PATH_LENGTH) * sizeof(VMWCHAR));

    filepath_buf[i++] = drv;
    filepath_buf[i++] = ':';
    if (filepath[0] != '/')
        filepath_buf[i++] = '/';

    if (vm_chset_ascii_to_ucs2(filepath_buf + i, (VM_FS_MAX_PATH_LENGTH - i) * sizeof(VMWCHAR), filepath) < 0)
        return false;

    i = 0;
    while (filepath_buf[i])
    {
        if (filepath_buf[i] == '/')
            filepath_buf[i] = '\\';
        i++;
    }

#ifdef LINKITSTORAGE_DEBUG
    Serial.print("[conv1]");
    _printwstr(filepath_buf);
#endif

    return true;
}

static boolean _conv_path_back(VMCWSTR filepath, VMSTR filepath_buf)
{
    int i = 0;

    memset(filepath_buf, 0, (VM_FS_MAX_PATH_LENGTH) * sizeof(char));

    if (vm_chset_ucs2_to_ascii(filepath_buf, (VM_FS_MAX_PATH_LENGTH) * sizeof(char), filepath + 2) < 0)
        return false;

    while (filepath_buf[i])
    {
        if (filepath_buf[i] == '\\')
            filepath_buf[i] = '/';
        i++;
    }

#ifdef LINKITSTORAGE_DEBUG
    Serial.print("[conv2]");
    LSLOG(filepath_buf);
#endif

    return true;
}

VM_FS_HANDLE linkit_file_open(const VMWSTR filename, VMUINT mode)
{
    if (mode == FILE_READ)
    {
        return vm_fs_open(filename, VM_FS_MODE_READ, TRUE);
    }
    else if (mode == FILE_WRITE)
    {
        VM_FS_ATTRIBUTE attr;
        VM_RESULT result = vm_fs_get_attributes(filename, &attr);
        if (VM_IS_SUCCEEDED(result))
        {
            return vm_fs_open(filename, VM_FS_MODE_CREATE_ALWAYS_WRITE, TRUE);
        }
        else
        {
            VM_FS_HANDLE fd = vm_fs_open(filename, VM_FS_MODE_WRITE, TRUE);
            vm_fs_seek(fd, 0, VM_FS_BASE_BEGINNING);
            return fd;
        }
    }
    else
        return -1;
}

/*****************************************************************************
 *
 * LFile MMI part (running on MMI thread)
 *
 *****************************************************************************/

boolean linkit_file_read_handler(void *userdata)
{
    linkit_file_read_struct *data = (linkit_file_read_struct *)userdata;
    VMUINT read;

    data->result = vm_fs_read(HDL(data->fd), data->buf, data->nbyte, &read);

    if (data->peek_mode)
    {
        // peek mode, rewind back
        vm_fs_seek(HDL(data->fd), -read, VM_FS_BASE_CURRENT);
    }

    return true;
}

boolean linkit_file_seek_handler(void *userdata)
{
    linkit_file_seek_struct *data = (linkit_file_seek_struct *)userdata;

    data->result = vm_fs_seek(HDL(data->fd), data->pos, VM_FS_BASE_BEGINNING);

    return true;
}

boolean linkit_file_position_handler(void *userdata)
{
    linkit_file_general_struct *data = (linkit_file_general_struct *)userdata;
    VMUINT pos = 0;
    data->result = vm_fs_get_position(HDL(data->fd), &pos);
    if (VM_IS_SUCCEEDED(data->result))
        data->value = pos;
    else
        data->value = 0;

    return true;
}

boolean linkit_file_size_handler(void *userdata)
{
    linkit_file_general_struct *data = (linkit_file_general_struct *)userdata;
    VMUINT size;

    data->result = vm_fs_get_size(HDL(data->fd), &size);
    if (VM_IS_SUCCEEDED(data->result))
        data->value = size;
    else
        data->value = 0;

    return true;
}

boolean linkit_file_close_handler(void *userdata)
{
    linkit_file_general_struct *data = (linkit_file_general_struct *)userdata;

    REF(data->fd)
    --;
    if (REF(data->fd) == 0)
    {
        vm_fs_close(HDL(data->fd));
        free((void *)data->fd);
        data->fd = 0;
    }

    return true;
}

boolean linkit_file_available_handler(void *userdata)
{
    linkit_file_general_struct *data = (linkit_file_general_struct *)userdata;

    VMUINT size = 0;
    VMUINT pos = 0;

    vm_fs_get_size(HDL(data->fd), &size);
    VMINT result = vm_fs_get_position(HDL(data->fd), &pos);

    if (!size || result < 0)
    {
        data->result = pos;
    }
    else
    {
        size -= pos;
        data->result = size > 0x7FFF ? 0x7FFF : size; // follow Arduino File.cpp's rule
    }

    return true;
}

boolean linkit_file_flush_handler(void *userdata)
{
    linkit_file_flush_struct *data = (linkit_file_flush_struct *)userdata;

    if (data->nbyte)
    {
        VMUINT written;
        vm_fs_write(HDL(data->fd), data->buf, data->nbyte, &written);
    }

    data->result = vm_fs_flush(HDL(data->fd));

    return true;
}

boolean linkit_file_find_handler(void *userdata)
{
    linkit_file_find_struct *data = (linkit_file_find_struct *)userdata;
    VMWCHAR filepath_buf[VM_FS_MAX_PATH_LENGTH];
    vm_fs_info_t info;
    VM_FS_ATTRIBUTE attr;
    VMINT findhdl;

    data->result = -1;

    if (!_conv_path(data->drv, data->findpath, filepath_buf))
        return true;

    int len = vm_wstr_string_length(filepath_buf);
    if (filepath_buf[len - 1] != '\\')
    {
        filepath_buf[len] = '\\';
        filepath_buf[len + 1] = 0;
        len++;
    }

    if (!data->findhdl)
    {
        filepath_buf[len] = '*';
        filepath_buf[len + 1] = 0;

#ifdef LINKITSTORAGE_DEBUG
        Serial.print("[find]");
        _printwstr(filepath_buf);
#endif

        findhdl = vm_fs_find_first(filepath_buf, &info);
        if (findhdl < 0)
            return true;

        // skip . and ..
        while ((info.filename[0] == '.' && info.filename[1] == 0) ||
               (info.filename[0] == '.' && info.filename[1] == '.' && info.filename[2] == 0))
        {
            data->result = vm_fs_find_next(findhdl, &info);
            if (VM_IS_FAILED(data->result))
            {
                vm_fs_find_close(findhdl);
                return true;
            }
        }

        data->findhdl = (VMUINT)malloc(sizeof(linkit_file_handle_struct));
        HDL(data->findhdl) = findhdl;
        REF(data->findhdl) = 1;
    }
    else
    {
        findhdl = HDL(data->findhdl);
        data->result = vm_fs_find_next(findhdl, &info);
        if (VM_IS_FAILED(data->result))
            return true;
    }

    vm_wstr_copy(filepath_buf + len, info.filename);

    VM_RESULT result = vm_fs_get_attributes(filepath_buf, &attr);
    if (VM_IS_SUCCEEDED(result) && attr & VM_FS_ATTRIBUTE_DIRECTORY)
    {
        LSLOG("[find]dir");
        data->is_dir = true;
        data->result = 0;
        data->fd = 0;
    }
    else if (VM_IS_FAILED(result)) // special case for SD label entry
    {
#ifdef LINKITSTORAGE_DEBUG
        Serial.print("[find]SD label?:");
        _printwstr(filepath_buf);
#endif
        data->is_dir = true;
        data->result = 0;
        data->fd = 0;
    }
    else
    {
        LSLOG("[find]file");
        data->is_dir = false;
        data->result = linkit_file_open(filepath_buf, data->mode);
        if (data->result < 0)
        {
            REF(data->findhdl)
            --;
            if (REF(data->findhdl) == 0)
            {
                vm_fs_find_close(HDL(data->findhdl));
                free((void *)data->findhdl);
                data->findhdl = 0;
            }
        }
        else
        {
            data->fd = (VMUINT)malloc(sizeof(linkit_file_handle_struct));
            HDL(data->fd) = data->result;
            REF(data->fd) = 1;
        }
    }
    _conv_path_back(filepath_buf, data->name);

    return true;
}

boolean linkit_file_find_close_handler(void *userdata)
{
    linkit_file_general_struct *data = (linkit_file_general_struct *)userdata;

    REF(data->fd)
    --;
    if (REF(data->fd) == 0)
    {
        vm_fs_find_close(HDL(data->fd));
        free((void *)data->fd);
        data->fd = 0;
    }

    return true;
}

/*****************************************************************************
 *
 * LFile MMI part (running on MMI thread)
 *
 *****************************************************************************/

static int recur_mkdir(VMWCHAR *path)
{
    VM_RESULT result;
    VMWCHAR *pos;
    VM_FS_ATTRIBUTE attr;

    // check if already exist
    result = vm_fs_get_attributes(path, &attr);
    if (VM_IS_SUCCEEDED(result))
        return -1; // already exist

    pos = path + 3;
    while (*pos)
    {
        if (*pos == '\\')
        {
            *pos = 0;
            vm_fs_create_directory(path);
            *pos = '\\';
        }
        pos++;
    }
    vm_fs_create_directory(path);

    // check if final path exist
    result = vm_fs_get_attributes(path, &attr);
    if (VM_IS_SUCCEEDED(result))
        result = 0; // succeed

    return result;
}

boolean linkit_drv_general_handler(void *userdata)
{
    linkit_drv_general_op_struct *data = (linkit_drv_general_op_struct *)userdata;
    VMWCHAR filepath_buf[VM_FS_MAX_PATH_LENGTH];
    VM_RESULT result;

    data->result = false;

    if (!_conv_path(data->drv, data->filepath, filepath_buf))
        return true;

    switch (data->op)
    {
    case 1: // exists
        VM_FS_ATTRIBUTE attr;
        result = vm_fs_get_attributes(filepath_buf, &attr);
        break;

    case 2: // mkdir
        result = recur_mkdir(filepath_buf);
        break;

    case 3: // remove
        result = vm_fs_delete(filepath_buf);
        break;

    case 4: // rmdir
        result = vm_fs_remove_directory(filepath_buf);
        break;
    }

#ifdef LINKITSTORAGE_DEBUG
    Serial.print("[gen_op]");
    Serial.print(data->op);
    Serial.print(":");
    Serial.println(result);
#endif

    data->result = result < 0 ? false : true;
    return true;
}

boolean linkit_drv_read_handler(void *userdata)
{
    linkit_drv_open_struct *data = (linkit_drv_open_struct *)userdata;
    VMWCHAR filepath_buf[VM_FS_MAX_PATH_LENGTH];
    VM_FS_ATTRIBUTE attr;
    VMINT fd;

    data->result = false;

    if (!_conv_path(data->drv, data->filepath, filepath_buf))
    {
        return true;
    }

    // identify if this is a file or dir
    VM_RESULT result = vm_fs_get_attributes(filepath_buf, &attr);

    if (VM_IS_SUCCEEDED(result) && attr & VM_FS_ATTRIBUTE_DIRECTORY)
    {
        data->is_dir = true;
        data->fd = 0;
        data->result = true;
        LSLOG("open ok (dir)");
    }
    else
    {
        data->is_dir = false;
        fd = linkit_file_open(filepath_buf, data->mode);
        if (fd > 0)
        {
            LSLOG("open ok (file)");
            data->result = true;

            data->fd = (VMUINT)malloc(sizeof(linkit_file_handle_struct));
            HDL(data->fd) = fd;
            REF(data->fd) = 1;
        }
    }

    return true;
}