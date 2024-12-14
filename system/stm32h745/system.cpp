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

#include "cmsis_os2.h"

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

//-----------------------------------------------------------------------------
/* syscalls */

// TODO: Resolve problem with sharing USART in DUAL_CORE_APP:
//    a) Use HSEM to manage access to stdio USART
// -> b) Allow access to USART by only one core (CM4)

#if !(defined(DUAL_CORE_APP) && defined(CORE_CM7))
#ifdef HAL_SYSTEM_RTOS_ENABLED
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

