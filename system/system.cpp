/*
 * system_init.c
 *
 *  Created on: 19 paź 2020
 *      Author: kwarc
 */


#include <cstdio>
#include <cerrno>
#include <cassert>

#include <cmsis/stm32f7xx.h>
#include <hal/hal_usart.hpp>

#include "cmsis_os2.h"

uint32_t SystemCoreClock = 16000000;

//-----------------------------------------------------------------------------
/* system_init function (used by startup code, called just before main) */

extern "C" void system_init(void)
{
#if (__FPU_USED == 1)
    /* Set bits 20-23 to enable full access to CP10 and CP11 coprocessors. */
    SCB->CPACR |= (3UL << 20) | (3UL << 22);
    __DSB();
    __ISB();
#endif
}

//-----------------------------------------------------------------------------
/* syscalls */

/* Redirect stdout & stdin to USART */

static osMutexId_t stdio_mutex_id = NULL;
static const osMutexAttr_t stdio_mutex_attr =
{
    "stdio_mutex",                            // human readable mutex name
    osMutexRecursive | osMutexPrioInherit,    // attr_bits
    NULL,                                     // memory for control block
    0U                                        // size for control block
};

extern "C" ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt)
{
    /* If doesnt exist, create mutex for stdio USART */
    stdio_mutex_id = (stdio_mutex_id == NULL) ? osMutexNew(&stdio_mutex_attr) : stdio_mutex_id;
    assert(stdio_mutex_id != NULL);

    size_t ret = 0;
    if (osMutexAcquire(stdio_mutex_id, osWaitForever) == osOK)
    {
        auto &stdio = hal::usart::stdio::get_instance();
        ret = stdio.write(reinterpret_cast<const std::byte*>(buf), cnt);
        ptr->_errno = (ret != cnt) ? EIO : 0;
    }
    else
    {
        ptr->_errno = EBUSY;
    }

    osMutexRelease(stdio_mutex_id);
    return ret;
}

extern "C" ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t cnt)
{
    /* If doesnt exist, create mutex for stdio USART */
    stdio_mutex_id = (stdio_mutex_id == NULL) ? osMutexNew(&stdio_mutex_attr) : stdio_mutex_id;
    assert(stdio_mutex_id != NULL);

    size_t ret = 0;
    if (osMutexAcquire(stdio_mutex_id, osWaitForever) == osOK)
    {
        auto &stdio = hal::usart::stdio::get_instance();
        ret = stdio.read(reinterpret_cast<std::byte*>(buf), cnt);
        ptr->_errno = (ret != cnt) ? EIO : 0;
    }
    else
    {
        ptr->_errno = EBUSY;
    }

    osMutexRelease(stdio_mutex_id);
    return ret;
}

// Non re-entrant versions
//extern "C" int _write (int fd, char *buf, int cnt)
//{
//    auto &stdio = hal::usart::stdio::get_instance();
//    return stdio.write(reinterpret_cast<const std::byte*>(buf), cnt);
//}
//
//extern "C" int _read (int fd, char *buf, int cnt)
//{
//    auto &stdio = hal::usart::stdio::get_instance();
//    return stdio.read(reinterpret_cast<std::byte*>(buf), cnt);
//}

