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

namespace
{

inline constexpr LTDC_Layer_TypeDef *get_layer_reg(ltdc::layer::id layer)
{
    return LTDC_Layer1 + (LTDC_Layer2 - LTDC_Layer1) * static_cast<uint8_t>(layer);
}

}

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

    LTDC->GCR |= cfg.pol.hsync << LTDC_GCR_HSPOL_Pos
              |  cfg.pol.vsync << LTDC_GCR_VSPOL_Pos
              |  cfg.pol.de << LTDC_GCR_DEPOL_Pos
              |  cfg.pol.pixel_clk << LTDC_GCR_PCPOL_Pos;

    LTDC->BCCR = (cfg.r << LTDC_BCCR_BCRED_Pos)
               | (cfg.g << LTDC_BCCR_BCGREEN_Pos)
               | (cfg.b << LTDC_BCCR_BCBLUE_Pos);

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

void ltdc::layer::configure(id layer, const layer::cfg &cfg)
{
    auto layer_reg = get_layer_reg(layer);

    uint32_t tmp = (LTDC->BPCR & LTDC_BPCR_AHBP) >> LTDC_BPCR_AHBP_Pos;
    layer_reg->WHPCR = (tmp + cfg.h_stop) << LTDC_LxWHPCR_WHSPPOS_Pos
                     | (tmp + cfg.h_start + 1) << LTDC_LxWHPCR_WHSTPOS_Pos;

    tmp = (LTDC->BPCR & LTDC_BPCR_AVBP) >> LTDC_BPCR_AVBP_Pos;
    layer_reg->WVPCR = (tmp + cfg.v_stop) << LTDC_LxWVPCR_WVSPPOS_Pos
                     | (tmp + cfg.v_start + 1) << LTDC_LxWVPCR_WVSTPOS_Pos;

    layer_reg->PFCR = static_cast<uint32_t>(cfg.pix_fmt) << LTDC_LxPFCR_PF_Pos;
    layer_reg->CACR = cfg.const_alpha_blend << LTDC_LxCACR_CONSTA_Pos;

    layer_reg->DCCR = cfg.a << LTDC_LxDCCR_DCALPHA_Pos
                    | cfg.r << LTDC_LxDCCR_DCRED_Pos
                    | cfg.g << LTDC_LxDCCR_DCGREEN_Pos
                    | cfg.b << LTDC_LxDCCR_DCBLUE_Pos;

    layer_reg->CFBAR = reinterpret_cast<uint32_t>(cfg.frame_buf_addr);
    layer_reg->CFBLR = (cfg.frame_buf_width * pixel_size.at(cfg.pix_fmt) + 3) << LTDC_LxCFBLR_CFBLL_Pos
                     | (cfg.frame_buf_width * pixel_size.at(cfg.pix_fmt)) << LTDC_LxCFBLR_CFBP_Pos;
    layer_reg->CFBLNR = cfg.frame_buf_height << LTDC_LxCFBLNR_CFBLNBR_Pos;

    LTDC->SRCR |= LTDC_SRCR_VBR;
}

void ltdc::layer::enable(id layer, bool layer_enable, bool color_keying_enable, bool clut_enable)
{
    auto layer_reg = get_layer_reg(layer);

    layer_reg->CR = clut_enable << LTDC_LxCR_CLUTEN_Pos
                  | color_keying_enable << LTDC_LxCR_COLKEN_Pos
                  | layer_enable << LTDC_LxCR_LEN_Pos;

    LTDC->SRCR |= LTDC_SRCR_VBR;
}

void ltdc::layer::set_framebuf_addr(id layer, void *addr)
{
    auto layer_reg = get_layer_reg(layer);
    layer_reg->CFBAR = reinterpret_cast<uint32_t>(addr);
    LTDC->SRCR |= LTDC_SRCR_VBR;
}