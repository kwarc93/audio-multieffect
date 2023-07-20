/*
 * ltdc.cpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#include "ltdc.hpp"

#include <cmsis/stm32f7xx.h>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

void ltdc::global_toggle(bool state)
{
    if (state)
        LTDC->GCR |= LTDC_GCR_LTDCEN;
    else
        LTDC->GCR &= ~LTDC_GCR_LTDCEN;
}

//-----------------------------------------------------------------------------
/* public */

void ltdc::configure(const cfg &cfg)
{
    rcc::enable_periph_clock(RCC_PERIPH_BUS(APB2, LTDC), true);

    LTDC->SSCR = (cfg.h.sync - 1) << LTDC_SSCR_HSW_Pos
               | (cfg.v.sync - 1) << LTDC_SSCR_VSH_Pos;

    LTDC->BPCR = (cfg.h.sync + cfg.h.back_porch - 1) << LTDC_BPCR_AHBP_Pos
               | (cfg.v.sync + cfg.v.back_porch - 1) << LTDC_BPCR_AVBP_Pos;

    LTDC->AWCR = (cfg.h.sync + cfg.h.back_porch + cfg.h.width - 1) << LTDC_AWCR_AAW_Pos
               | (cfg.v.sync + cfg.v.back_porch + cfg.v.height - 1) << LTDC_AWCR_AAH_Pos;

    LTDC->TWCR = (cfg.h.sync + cfg.h.back_porch + cfg.h.width + cfg.h.front_porch - 1) << LTDC_TWCR_TOTALW_Pos
               | (cfg.v.sync + cfg.v.back_porch + cfg.v.height + cfg.v.front_porch - 1) << LTDC_TWCR_TOTALH_Pos;

    LTDC->GCR &= ~(LTDC_GCR_HSPOL_Msk | LTDC_GCR_VSPOL_Msk | LTDC_GCR_DEPOL_Msk | LTDC_GCR_PCPOL_Msk);

    LTDC->GCR |= cfg.h.sync_polarity << LTDC_GCR_HSPOL_Pos
              |  cfg.v.sync_polarity << LTDC_GCR_VSPOL_Pos
              |  cfg.de_pol << LTDC_GCR_DEPOL_Pos
              |  cfg.pixel_clk_pol << LTDC_GCR_PCPOL_Pos;

    LTDC->BCCR = (cfg.bkgd_col_r << LTDC_BCCR_BCRED_Pos)
               | (cfg.bkgd_col_g << LTDC_BCCR_BCGREEN_Pos)
               | (cfg.bkgd_col_b << LTDC_BCCR_BCBLUE_Pos);

    if (cfg.err_irq_enable)
    {
        LTDC->IER |= LTDC_IER_FUIE | LTDC_IER_TERRIE;
    }
}

void ltdc::enable(bool state)
{
    if (state)
    {
        rcc::enable_periph_clock(RCC_PERIPH_BUS(APB2, LTDC), true);

        /* IRQ priority level */
        IRQn_Type nvic_irq = static_cast<IRQn_Type>(LTDC_ER_IRQn);
        NVIC_SetPriority(nvic_irq, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 15, 0 ));

        /* Enable IRQ */
        NVIC_EnableIRQ(nvic_irq);

        global_toggle(true);
    }
    else
    {
        global_toggle(false);

        /* Disable IRQ */
        IRQn_Type nvic_irq = static_cast<IRQn_Type>(LTDC_ER_IRQn);
        NVIC_ClearPendingIRQ(nvic_irq);
        NVIC_DisableIRQ(nvic_irq);

        LTDC->IER = 0;

        rcc::enable_periph_clock(RCC_PERIPH_BUS(APB2, LTDC), false);
    }
}

void ltdc::irq_handler(void)
{
    /* FIFO Underrun error */
    if (LTDC->ISR & LTDC_ISR_FUIF)
    {
        LTDC->ICR |= LTDC_ICR_CFUIF;

        asm volatile ("BKPT 0");
    }

    /* Transfer error */
    if (LTDC->ISR & LTDC_ISR_TERRIF)
    {
        LTDC->ICR |= LTDC_ICR_CTERRIF;

        asm volatile ("BKPT 0");
    }
}

void ltdc::layer::configure(id layer, const cfg &cfg)
{

}

void ltdc::layer::enable(id layer, bool state)
{
    if (state)
    {

    }
    else
    {

    }
}
