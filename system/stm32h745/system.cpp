/*
 * system_init.c
 *
 *  Created on: 19 paź 2020
 *      Author: kwarc
 */


#include <cstdio>
#include <cerrno>
#include <cassert>

#include <cmsis/stm32h7xx.h>
#include <hal_system.hpp>
#include <hal_usart.hpp>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

uint32_t SystemCoreClock = 64000000;

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

    /* SEVONPEND enabled so that an interrupt coming from the CPU(n) interrupt signal is
       detectable by the CPU after a WFI/WFE instruction.*/
   SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

#ifdef CORE_CM7
    /*
     * Disable the FMC bank1 (enabled after reset).
     * This, prevents CPU speculation access on this bank which blocks the use of FMC during
     * 24us. During this time the others FMC master (such as LTDC) cannot use it!
     */
    FMC_Bank1_R->BTCR[0] &= ~FMC_BCRx_MBKEN;

    /* Enable CortexM7 HSEM EXTI line (line 78)*/
    EXTI_D2->EMR3 |= 0x4000UL;

    /* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
    if((DBGMCU->IDCODE & 0xFFFF0000U) < 0x20000000U)
    {
        /* if stm32h7 revY*/
        *((__IO uint32_t*)0x51008108) = 0x00000001U;
    }

    SCB->VTOR = FLASH_BANK1_BASE;
#endif /* CORE_CM7 */
#ifdef CORE_CM4
    SCB->VTOR = FLASH_BANK2_BASE;
#endif /* CORE_CM4 */
}

/* Inter-Processor Communication (IPC) handlers */
#ifdef DUAL_CORE_APP
extern "C" void ipc_notify_core(void * xUpdatedMessageBuffer)
{
    MessageBufferHandle_t updated_buffer = static_cast<MessageBufferHandle_t>(xUpdatedMessageBuffer);

    if (updated_buffer == hal::ipc::ipc_struct.cm4_to_cm7.mb_handle)
    {
        hal::ipc::notify_cm7();
    }
    else if (updated_buffer == hal::ipc::ipc_struct.cm7_to_cm4.mb_handle)
    {
        hal::ipc::notify_cm4();
    }
}
#endif /* DUAL_CORE_APP */

//-----------------------------------------------------------------------------
/* syscalls */

__attribute__((noreturn))
void abort(void)
{
    __disable_irq();
    __BKPT(0);
    for (;;);
}

#ifdef HAL_SYSTEM_RTOS_ENABLED
/* Override default lock/unlock functions to let the heap be thread-safe */
void __malloc_lock(struct _reent *r)
{
    assert(!xPortIsInsideInterrupt());
    vTaskSuspendAll();
}

void __malloc_unlock(struct _reent *r)
{
    xTaskResumeAll();
}
#endif /* HAL_SYSTEM_RTOS_ENABLED */

// TODO: Resolve problem with stdio USART in DUAL_CORE_APP:
//    a) Use HSEM to synchronize access to stdio USART
// -> b) Limit access to USART to only one core (CM4)
//    c) Use another USART instance for second core

#if !(defined(DUAL_CORE_APP) && defined(CORE_CM7))
#ifdef HAL_SYSTEM_RTOS_ENABLED
static SemaphoreHandle_t stdio_mutex_id = NULL;

extern "C" ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt)
{
    assert(!xPortIsInsideInterrupt());

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
    assert(!xPortIsInsideInterrupt());

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
#endif /* !(defined(DUAL_CORE_APP) && defined(CORE_CM7)) */

//-----------------------------------------------------------------------------
/* FreeRTOS hooks */

#ifdef HAL_SYSTEM_RTOS_ENABLED
extern "C"
{
void vApplicationIdleHook (void){ __WFI(); }
void vApplicationMallocFailedHook (void) { assert(0); }
void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName) { assert(0); }
}
#endif /* HAL_SYSTEM_RTOS_ENABLED */

