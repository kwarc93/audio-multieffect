/*
 * system.cpp
 *
 *  Created on: 19 paź 2020
 *      Author: kwarc
 */


#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>

#include <cmsis/stm32f7xx.h>
#include <hal_system.hpp>
#include <hal_usart.hpp>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

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

    /* Enable 'FPU Flush To Zero' to avoid handling subnormals */
    FPU->FPDSCR |= (1UL << FPU_FPDSCR_FZ_Pos);
    __set_FPSCR(__get_FPSCR() | (1UL << FPU_FPDSCR_FZ_Pos));
#endif

    /* Enable fault handlers and additional traps */
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk;
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

    /*
     * Disable the FMC bank1 (enabled after reset).
     * This, prevents CPU speculation access on this bank which blocks the use of FMC during
     * 24us. During this time the others FMC master (such as LTDC) cannot use it!
     */
    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;
    FMC_Bank1->BTCR[0] &= ~FMC_BCR1_MBKEN;
    RCC->AHB3ENR &= ~RCC_AHB3ENR_FMCEN;
}

extern "C" [[noreturn]] void abort(void)
{
    puts("abort");
    __disable_irq();
    __BKPT(0);
    for (;;);
}

//-----------------------------------------------------------------------------
/* syscalls */

extern "C" caddr_t _sbrk(int incr)
{
    extern char __HeapBase[];
    extern char __StackLimit[];

    static auto heap_end = __HeapBase;
    
    /* Check heap collision against maximum allowed stack limit */
    if (heap_end + incr > __StackLimit)
    {
        errno = ENOMEM;
        assert(!"_sbrk: no memory");
        return reinterpret_cast<caddr_t>(-1);
    }

    const auto prev = heap_end;
    heap_end += incr;
    return reinterpret_cast<caddr_t>(prev);
}

#ifdef HAL_SYSTEM_RTOS_ENABLED
/* Override default lock/unlock functions to let the heap be thread-safe */
extern "C" void __malloc_lock(struct _reent *r)
{
    assert(!xPortIsInsideInterrupt());
    vTaskSuspendAll();
}

extern "C" void __malloc_unlock(struct _reent *r)
{
    xTaskResumeAll();
}

static SemaphoreHandle_t stdio_mutex_id = NULL;

extern "C" ssize_t _write(int fd, const void *buf, size_t cnt)
{
    if (xPortIsInsideInterrupt())
    {
        errno = EACCES;
        return -1; 
    }

    stdio_mutex_id = (stdio_mutex_id == nullptr) ? xSemaphoreCreateRecursiveMutex() : stdio_mutex_id;
    assert(stdio_mutex_id != nullptr);

    errno = EBUSY;
    size_t ret = 0;
    if (xSemaphoreTakeRecursive(stdio_mutex_id, portMAX_DELAY) == pdTRUE)
    {
        auto &stdio = hal::usart::stdio::get_instance();
        ret = stdio.write(reinterpret_cast<const std::byte*>(buf), cnt);
        errno = (ret != cnt) ? EIO : 0;
        xSemaphoreGiveRecursive(stdio_mutex_id);
    }

    return ret;
}

extern "C" ssize_t _read(int fd, void *buf, size_t cnt)
{
    if (xPortIsInsideInterrupt())
    {
        errno = EACCES;
        return -1; 
    }

    stdio_mutex_id = (stdio_mutex_id == nullptr) ? xSemaphoreCreateRecursiveMutex() : stdio_mutex_id;
    assert(stdio_mutex_id != nullptr);

    errno = EBUSY;
    size_t ret = 0;
    if (xSemaphoreTakeRecursive(stdio_mutex_id, portMAX_DELAY) == pdTRUE)
    {
        auto &stdio = hal::usart::stdio::get_instance();
        ret = stdio.read(reinterpret_cast<std::byte*>(buf), cnt);
        errno = (ret != cnt) ? EIO : 0;
        xSemaphoreGiveRecursive(stdio_mutex_id);
    }

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

//-----------------------------------------------------------------------------
/* FreeRTOS hooks */

#ifdef HAL_SYSTEM_RTOS_ENABLED
extern "C"
{
void vApplicationIdleHook (void){ __WFI(); }
void vApplicationMallocFailedHook (void) { assert(!"vApplicationMallocFailedHook"); }
void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName) { assert(!"vApplicationStackOverflowHook"); }
}
#endif /* HAL_SYSTEM_RTOS_ENABLED */
