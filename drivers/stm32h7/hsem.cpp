/*
 * hsem.cpp
 *
 *  Created on: 30 lis 2024
 *      Author: kwarc
 */

#include "hsem.hpp"

#include <cmsis/stm32h7xx.h>

#include <drivers/stm32h7/core.hpp>
#include <drivers/stm32h7/rcc.hpp>

#include <cassert>

using namespace drivers;

void hsem::init(void)
{
    drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB4, HSEM), true);
#ifdef DUAL_CORE
    const auto irq = core::get_current_cpu_id() == core::core_id::cortex_m7 ? HSEM1_IRQn : HSEM2_IRQn;
#else
    const auto irq = HSEM_IRQn;
#endif
    NVIC_SetPriority(irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(irq);
}

bool hsem::take(uint8_t id, uint8_t pid)
{
    assert(id <= HSEM_SEMID_MAX);
    assert(pid > 0);

    /* 1. Write R register with MasterID, processID and take bit=1 */
    /* 2. Read the R register. Take achieved if MasterID and processID match and take bit set to 1 */
#ifdef DUAL_CORE_SHARED_CODE
    const uint32_t core_id = core::get_current_cpu_id() == core::core_id::cortex_m7 ? HSEM_CPU1_COREID : HSEM_CPU2_COREID;
    HSEM->R[id] = ((pid & HSEM_R_PROCID) | ((core_id << POSITION_VAL(HSEM_R_COREID)) & HSEM_R_COREID) | HSEM_R_LOCK);
    return HSEM->R[id] == ((pid & HSEM_R_PROCID) | ((core_id << POSITION_VAL(HSEM_R_COREID)) & HSEM_R_COREID) | HSEM_R_LOCK);
#else
    HSEM->R[id] = (pid | HSEM_CR_COREID_CURRENT | HSEM_R_LOCK);
    return HSEM->R[id] == (pid | HSEM_CR_COREID_CURRENT | HSEM_R_LOCK);
#endif /* DUAL_CORE_SHARED_CODE */
}

bool hsem::fast_take(uint8_t id)
{
    assert(id <= HSEM_SEMID_MAX);

    /* Read the RLR register to take the semaphore */
#ifdef DUAL_CORE_SHARED_CODE
    const uint32_t core_id = core::get_current_cpu_id() == core::core_id::cortex_m7 ? HSEM_CPU1_COREID : HSEM_CPU2_COREID;
    return HSEM->RLR[id] == (((core_id << POSITION_VAL(HSEM_R_COREID)) & HSEM_RLR_COREID) | HSEM_RLR_LOCK);
#else
    return HSEM->RLR[id] == (HSEM_CR_COREID_CURRENT | HSEM_RLR_LOCK);
#endif /* DUAL_CORE_SHARED_CODE */
}

bool hsem::is_taken(uint8_t id)
{
    assert(id <= HSEM_SEMID_MAX);
    return (HSEM->R[id] & HSEM_R_LOCK) != 0;
}

void hsem::release(uint8_t id, uint8_t pid)
{
    assert(id <= HSEM_SEMID_MAX);

    /* Clear the semaphore by writing to the R register : the MasterID , the processID and take bit = 0  */
#ifdef DUAL_CORE_SHARED_CODE
    const uint32_t core_id = core::get_current_cpu_id() == core::core_id::cortex_m7 ? HSEM_CPU1_COREID : HSEM_CPU2_COREID;
    HSEM->R[id] = (pid | ((core_id << POSITION_VAL(HSEM_R_COREID)) & HSEM_R_COREID));
#else
    HSEM->R[id] = (pid | HSEM_CR_COREID_CURRENT);
#endif /* DUAL_CORE_SHARED_CODE */
}

void hsem::enable_notification(uint8_t id)
{
    assert(id <= HSEM_SEMID_MAX);
    const uint32_t sem_mask = 1u << id;
#ifdef DUAL_CORE_SHARED_CODE
    /* Enable the semaphore mask interrupts */
    if (core::get_current_cpu_id() == core::core_id::cortex_m7)
    {
        HSEM->C1IER |= sem_mask;
    }
    else
    {
        HSEM->C2IER |= sem_mask;
    }
#else
    HSEM_COMMON->IER |= sem_mask;
#endif /* DUAL_CORE_SHARED_CODE */
}

void hsem::disable_notification(uint8_t id)
{
    assert(id <= HSEM_SEMID_MAX);
    const uint32_t sem_mask = 1u << id;
#ifdef DUAL_CORE_SHARED_CODE
    /* Disable the semaphore mask interrupts */
    if (core::get_current_cpu_id() == core::core_id::cortex_m7)
    {
        HSEM->C1IER &= ~sem_mask;
    }
    else
    {
        HSEM->C2IER &= ~sem_mask;
    }
#else
    HSEM_COMMON->IER &= ~sem_mask;
#endif /* DUAL_CORE_SHARED_CODE */
}

void hsem::irq_handler (void)
{
    uint32_t statusreg;
#ifdef DUAL_CORE_SHARED_CODE
    if (core::get_current_cpu_id() == core::core_id::cortex_m7)
    {
      /* Get the list of masked freed semaphores */
      statusreg = HSEM->C1MISR;

      /* Disable Interrupts */
      HSEM->C1IER &= ~statusreg;

      /* Clear Flags */
      HSEM->C1ICR = statusreg;
    }
    else
    {
      /* Get the list of masked freed semaphores */
      statusreg = HSEM->C2MISR;

      /* Disable Interrupts */
      HSEM->C2IER &= ~statusreg;

      /* Clear Flags */
      HSEM->C2ICR = statusreg;
    }
#else
    /* Get the list of masked freed semaphores */
    statusreg = HSEM_COMMON->MISR;

    /* Disable Interrupts */
    HSEM_COMMON->IER &= ~statusreg;

    /* Clear Flags */
    HSEM_COMMON->ICR = statusreg;

#endif /* DUAL_CORE_SHARED_CODE */
}

