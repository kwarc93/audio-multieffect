/*
 * exti.cpp
 *
 *  Created on: 11 maj 2024
 *      Author: kwarc
 */

#include "exti.hpp"

#include <cassert>

#include <cmsis/stm32h7xx.h>
#include <drivers/stm32h7/rcc.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */
void exti::configure(bool state, line line, port port, mode mode, edge edge, exti_cb_t callback)
{
    rcc::enable_periph_clock(RCC_PERIPH_BUS(APB4, SYSCFG), true);

    /* Calculate SYSCFG register for the EXTI line */
    const uint8_t line_nr = static_cast<uint8_t>(line);
    const uint8_t reg_id = line_nr / 4;

    /* Calculate EXTI bitfield shift and mask */
    const uint8_t shift = (line_nr % 4) * 4;
    const uint32_t mask = 0x000F << shift;

    /* Configure GPIO port source for the EXTI line */
    uint32_t temp = SYSCFG->EXTICR[reg_id] & ~mask;
    temp |= (static_cast<uint8_t>(port) << shift) & mask;
    SYSCFG->EXTICR[reg_id] = temp;

    /* Save callback */
    callbacks[line_nr] = callback;

    const uint32_t line_mask = (1 << line_nr);

    switch (mode)
    {
    case mode::interrupt:
        if (state)
            EXTI->IMR1 |= line_mask;
        else
            EXTI->IMR1 &= ~line_mask;
        break;
    case mode::event:
        if (state)
            EXTI->EMR1 |= line_mask;
        else
            EXTI->EMR1 &= ~line_mask;
        break;
    default:
        break;
    }

    switch (edge)
    {
    case edge::rising:
        EXTI->RTSR1 |= line_mask;
        break;
    case edge::falling:
        EXTI->FTSR1 |= line_mask;
        break;
    case edge::both:
        EXTI->RTSR1 |= line_mask;
        EXTI->FTSR1 |= line_mask;
        break;
    default:
        break;
    }

    /* Configure NVIC */
    IRQn_Type irq = EXTI0_IRQn;
    switch (line_nr)
    {
    case 0 ... 4:
        irq = static_cast<IRQn_Type>(irq + line_nr);
        break;
    case 5 ... 9:
        irq = EXTI9_5_IRQn;
        break;
    case 10 ... 15:
        irq = EXTI15_10_IRQn;
        break;
    default:
        break;
    }

    NVIC_SetPriority(irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(irq);
}

void exti::trigger(line line)
{
    EXTI->SWIER1 |= 1 << static_cast<uint8_t>(line);
}

void exti::irq_handler(line line)
{
    const uint32_t line_nr = static_cast<uint8_t>(line);
    const uint32_t line_mask = 1 << line_nr;

    if (EXTI->PR1 & line_mask)
    {
        /* Clear pending interrupt */
        EXTI->PR1 = line_mask;

        if (callbacks[line_nr])
            callbacks[line_nr]();
    }
}

void exti::irq_handler(line line_start, line line_end)
{
    const uint32_t line_start_nr = static_cast<uint8_t>(line_start);
    const uint32_t line_end_nr = static_cast<uint8_t>(line_end);

    for (unsigned line = line_start_nr; line <= line_end_nr; line++)
    {
        const uint32_t line_mask = 1 << line;

        if (EXTI->PR1 & line_mask)
        {
            /* Clear pending interrupt */
            EXTI->PR1 = line_mask;

            if (callbacks[line])
                callbacks[line]();
        }
    }
}
