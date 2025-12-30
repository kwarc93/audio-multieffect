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
#include <hal_system.hpp>
#include <hal_usart.hpp>

#include "FreeRTOS.h"
#include "semphr.h"

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

    /*
     * Disable the FMC bank1 (enabled after reset).
     * This, prevents CPU speculation access on this bank which blocks the use of FMC during
     * 24us. During this time the others FMC master (such as LTDC) cannot use it!
     */
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
    FMC_Bank1->BTCR[0] &= ~FMC_BCR1_MBKEN;
    RCC->AHB3ENR &= ~RCC_AHB3ENR_FMCEN;
}

//-----------------------------------------------------------------------------
/* syscalls */

#ifdef HAL_SYSTEM_RTOS_ENABLED
static SemaphoreHandle_t stdio_mutex_id = NULL;

extern "C" ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt)
{
    /* If doesnt exist, create mutex for stdio USART */
    stdio_mutex_id = (stdio_mutex_id == NULL) ? xSemaphoreCreateRecursiveMutex() : stdio_mutex_id;
    assert(stdio_mutex_id != NULL);

    size_t ret = 0;
    if (xSemaphoreTake(stdio_mutex_id, portMAX_DELAY) == pdTRUE)
    {
        auto &stdio = hal::usart::stdio::get_instance();
        ret = stdio.write(reinterpret_cast<const std::byte*>(buf), cnt);
        ptr->_errno = (ret != cnt) ? EIO : 0;
    }
    else
    {
        ptr->_errno = EBUSY;
    }

    xSemaphoreGive(stdio_mutex_id);
    return ret;
}

extern "C" ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t cnt)
{
    /* If doesnt exist, create mutex for stdio USART */
    stdio_mutex_id = (stdio_mutex_id == NULL) ? xSemaphoreCreateRecursiveMutex() : stdio_mutex_id;
    assert(stdio_mutex_id != NULL);

    size_t ret = 0;
    if (xSemaphoreTake(stdio_mutex_id, portMAX_DELAY) == pdTRUE)
    {
        auto &stdio = hal::usart::stdio::get_instance();
        ret = stdio.read(reinterpret_cast<std::byte*>(buf), cnt);
        ptr->_errno = (ret != cnt) ? EIO : 0;
    }
    else
    {
        ptr->_errno = EBUSY;
    }

    xSemaphoreGive(stdio_mutex_id);
    return ret;
}

#else
extern "C" int _write (int fd, char *buf, int cnt)
{
    auto &stdio = hal::usart::stdio::get_instance();
    return stdio.write(reinterpret_cast<const std::byte*>(buf), cnt);
}

extern "C" int _read (int fd, char *buf, int cnt)
{
    auto &stdio = hal::usart::stdio::get_instance();
    return stdio.read(reinterpret_cast<std::byte*>(buf), cnt);
}
#endif /* HAL_SYSTEM_RTOS_ENABLED */

