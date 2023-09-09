/*
 * dma2d.cpp
 *
 *  Created on: 25 lip 2023
 *      Author: kwarc
 */

#include "dma2d.hpp"

#include <cmsis/stm32f7xx.h>
#include <drivers/stm32f7/rcc.hpp>

#include <array>
#include <cstring>
#include <cassert>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

//-----------------------------------------------------------------------------
/* private */


void dma2d::set_mode(mode mode)
{
    static const std::array<uint32_t, 4> dma2d_cr_cmode
    {{
        0,
        DMA2D_CR_MODE_0,
        DMA2D_CR_MODE_1,
        (DMA2D_CR_MODE_0 | DMA2D_CR_MODE)
    }};

    DMA2D->CR &= ~DMA2D_CR_MODE_Msk;
    DMA2D->CR |= dma2d_cr_cmode[static_cast<uint32_t>(mode)];
}

void dma2d::send_command(command cmd)
{
    static const std::array<uint8_t, 3> dma2d_cr_cmd
    {{
        DMA2D_CR_ABORT,
        DMA2D_CR_SUSP,
        DMA2D_CR_START,
    }};

    DMA2D->CR &= ~(DMA2D_CR_ABORT_Msk | DMA2D_CR_SUSP_Msk | DMA2D_CR_START_Msk);
    DMA2D->CR |= dma2d_cr_cmd[static_cast<uint8_t>(cmd)];
}

//-----------------------------------------------------------------------------
/* public */

void dma2d::enable(bool state)
{
    if (state)
    {
        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, DMA2D), true);

        /* Transfer complete & configuration error interrupt enable. */
        DMA2D->CR |= DMA2D_CR_TCIE | DMA2D_CR_CEIE;

        NVIC_SetPriority(DMA2D_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
        NVIC_EnableIRQ(DMA2D_IRQn);
    }
    else
    {
        send_command(command::abort);

        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, DMA2D), false);

        /* Disable interrupt. */
        NVIC_DisableIRQ(DMA2D_IRQn);
        NVIC_ClearPendingIRQ(DMA2D_IRQn);
    }
}

void dma2d::set_ahb_dead_time(uint8_t dead_time)
{
    DMA2D->AMTCR = (dead_time << DMA2D_AMTCR_DT_Pos) | DMA2D_AMTCR_EN;
}

void dma2d::transfer(const transfer_cfg &cfg)
{
    /* Only 'mem_to_mem' and 'reg_to_mem' is currently supported */
    assert(cfg.transfer_mode == mode::mem_to_mem || cfg.transfer_mode == mode::reg_to_mem);

    /* Wait for DMA2D to be ready */
    while (DMA2D->CR & DMA2D_CR_START);

    /* Set transfer mode */
    set_mode(cfg.transfer_mode);

    /* Configure color parameters */
    uint32_t color = 0;
    memcpy(&color, cfg.src, pixel_size.at(cfg.color_mode));
    DMA2D->OCOLR = (cfg.alpha << 24) | color;

    /* Configure foreground memory parameters. */
    DMA2D->FGMAR = reinterpret_cast<uint32_t>(cfg.src);
    DMA2D->FGPFCCR = (static_cast<uint32_t>(cfg.color_mode) << DMA2D_FGPFCCR_CM_Pos)
                   | (cfg.alpha << DMA2D_FGPFCCR_ALPHA_Pos) | DMA2D_FGPFCCR_AM_0;
    DMA2D->FGOR = cfg.rotate_90_deg ? (cfg.x2 - cfg.x1) : 0;

    /* Configure output memory parameters. */
    DMA2D->OMAR = reinterpret_cast<uint32_t>(cfg.dst) + pixel_size.at(cfg.color_mode) * (cfg.y1 * cfg.width + cfg.x1);
    DMA2D->OPFCCR = static_cast<uint32_t>(cfg.color_mode) << DMA2D_FGPFCCR_CM_Pos;
    DMA2D->OOR = cfg.rotate_90_deg ? 0 : cfg.width - (cfg.x2 - cfg.x1 + 1);

    /* Set number of lines and pixels per line values. */
    DMA2D->NLR = (cfg.y2 - cfg.y1 + 1) << DMA2D_NLR_NL_Pos;
    DMA2D->NLR |= (cfg.rotate_90_deg ? 1 : (cfg.x2 - cfg.x1 + 1)) << DMA2D_NLR_PL_Pos;

    transfer_callback = cfg.rotate_90_deg ? nullptr : cfg.transfer_complete_cb;

    if (!cfg.rotate_90_deg)
    {
        send_command(command::start);
        return;
    }

    const size_t px_size = pixel_size.at(cfg.color_mode);
    int16_t lines = cfg.x2 - cfg.x1 + 1;
    int16_t x1 = cfg.x1;

    while (lines)
    {
        /* Update transfer mode */
        set_mode(cfg.transfer_mode);

        int16_t xd1 = cfg.y1;
        int16_t yd1 = cfg.width - x1 - 1;

        /* Update output memory address */
        DMA2D->OMAR = reinterpret_cast<uint32_t>(cfg.dst) + px_size * (yd1 * cfg.height + xd1);

        x1++;
        lines--;

        if (lines == 0)
            transfer_callback = cfg.transfer_complete_cb;

        send_command(command::start);

        /* Wait for DMA2D to be ready */
        while (DMA2D->CR & DMA2D_CR_START);

        /* Update foreground memory address */
        DMA2D->FGMAR += pixel_size.at(cfg.color_mode);
    }
}

void dma2d::irq_handler(void)
{
    if (DMA2D->ISR & DMA2D_ISR_TCIF)
    {
        DMA2D->IFCR = DMA2D_IFCR_CTCIF;

        if (transfer_callback)
            transfer_callback();
    }

    /* Configuration error */
    if (DMA2D->ISR & DMA2D_ISR_CEIF)
    {
        DMA2D->IFCR = DMA2D_IFCR_CCEIF;

        assert(!"DMA2D configuration error");
    }
}



