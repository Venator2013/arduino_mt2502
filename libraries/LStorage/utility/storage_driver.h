
#ifndef _STORAGE_DRIVER_H_
#define _STORAGE_DRIVER_H_

#ifdef __cplusplus
extern "C"
{
#endif



#ifndef FILE_READ
#define FILE_READ 0x01
#endif

#ifndef FILE_WRITE
#define FILE_WRITE 0x13
#endif

    /*****************************************************************************
     *
     * structure data between Arduino / MMI thread
     *
     *****************************************************************************/

    struct linkit_file_general_struct
    {
        VMUINT fd;
        VMINT result;
        VMUINT value;
    };

    struct linkit_file_flush_struct
    {
        VMUINT fd;
        VMINT result;
        void *buf;
        VMUINT nbyte;
    };

    struct linkit_file_read_struct
    {
        VMUINT fd;
        VMINT result;
        void *buf;
        VMUINT nbyte;
        boolean peek_mode;
    };

    struct linkit_file_seek_struct
    {
        VMUINT fd;
        VMINT result;
        VMINT pos;
    };

    struct linkit_file_find_struct
    {
        VMUINT findhdl;
        char *findpath;
        VMWCHAR drv;
        uint8_t mode;

        uint8_t is_dir;
        VMUINT fd;
        VMINT result; // 0: ok, <0: error
        char name[VM_FS_MAX_PATH_LENGTH];
    };

    boolean linkit_file_read_handler(void *userdata);
    boolean linkit_file_seek_handler(void *userdata);
    boolean linkit_file_position_handler(void *userdata);
    boolean linkit_file_size_handler(void *userdata);
    boolean linkit_file_close_handler(void *userdata);
    boolean linkit_file_available_handler(void *userdata);
    boolean linkit_file_flush_handler(void *userdata);
    boolean linkit_file_find_handler(void *userdata);
    boolean linkit_file_find_close_handler(void *userdata);

    struct linkit_drv_general_op_struct
    {
        const char *filepath;
        VMINT op;
        VMINT result;
        VMWCHAR drv;
    };

    struct linkit_drv_open_struct
    {
        const char *filepath;
        VMINT mode;
        VMINT result;
        VMUINT fd;
        VMINT is_dir;
        VMWCHAR drv;
    };

    boolean linkit_drv_general_handler(void *userdata);
    boolean linkit_drv_read_handler(void *userdata);

    struct linkit_file_handle_struct
    {
        VMINT _hdl;
        VMINT _ref;
    };

#define HDL(fd) ((linkit_file_handle_struct *)fd)->_hdl
#define REF(fd) ((linkit_file_handle_struct *)fd)->_ref

#ifdef __cplusplus
}
#endif

#endif