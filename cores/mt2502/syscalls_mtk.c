#include <stdio.h>
#if defined(__GNUC__) /* GCC CS3 */
#include <sys/stat.h>
#endif
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_pwm.h"
#include "vmlog.h"
#include "vmmemory.h"
#include "vmsystem.h"
#include "vmtype.h"

typedef VMINT (*vm_get_sym_entry_t)(char *symbol);
vm_get_sym_entry_t vm_get_sym_entry;

#define RESERVED_MEMORY_SIZE 600 * 1024

static unsigned int g_memory_size = 200 * 1024;
static unsigned int maxMem = 0;
static char *g_base_address = NULL;

int __g_errno = 0;

int *__errno()
{
    return &__g_errno;
}

extern caddr_t _sbrk(int incr)
{
    static char *heap = NULL;
    static char *base = NULL;
    char *prev_heap;

    if (heap == NULL)
    {
        base = g_base_address;
        if (base == NULL)
        {
            vm_log_fatal("malloc failed");
        }
        else
        {
            heap = base;
            vm_log_info("init memory success");
        }
    }

    if (heap + incr > g_base_address + g_memory_size)
    {
        vm_log_fatal("memory not enough");
        vm_pwr_reboot();
    }

    prev_heap = heap;

    heap += incr;
    maxMem -= incr;
    vm_log_info("heap reserved: %d bytes , %d bytes left", incr, maxMem);

    return (caddr_t)prev_heap;
}

extern int link(char *cOld, char *cNew)
{
    return -1;
}

extern int _close(int file)
{
    return -1;
}

extern int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

extern int _isatty(int file)
{
    return 1;
}

extern int _lseek(int file, int ptr, int dir)
{
    return 0;
}

extern int _read(int file, char *ptr, int len)
{
    return 0;
}

extern int _write(int file, char *ptr, int len)
{
    return len;
}

extern void _exit(int status)
{
    for (;;)
        ;
}

extern void _kill(int pid, int sig)
{
    return;
}

extern int _getpid(void)
{
    return -1;
}

int __cxa_guard_acquire(int *g)
{
    return !*(char *)(g);
}

void __cxa_guard_release(int *g)
{
    *(char *)g = 1;
}

void __cxa_pure_virtual(void)
{
    while (1)
        ;
}

void __cxa_deleted_virtual(void)
{

    while (1)
        ;
}

typedef void (**__init_array)(void);

void __libc_init_array(void);

void gcc_entry(unsigned int entry, unsigned int init_array_start, unsigned int count)
{
    __init_array ptr;
    vm_get_sym_entry = (vm_get_sym_entry_t)entry;

    unsigned int size = vm_pmng_get_total_heap_size();

    vm_log_info("max heap site %d , Reserved %d", vm_pmng_get_total_heap_size(), RESERVED_MEMORY_SIZE);

    if (size > RESERVED_MEMORY_SIZE)
    {
        g_memory_size = size - RESERVED_MEMORY_SIZE;
        maxMem = g_memory_size;
    }

    g_base_address = vm_malloc(g_memory_size);
    vm_log_info("heap size allocated %d", g_memory_size);

    ptr = (__init_array)init_array_start;
    for (unsigned int i = 1; i < count; i++)
    {
        ptr[i]();
    }

    vm_log_info("call vm_main");
    vm_main();
}
