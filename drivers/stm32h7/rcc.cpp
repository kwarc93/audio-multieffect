/*
 * rcc.cpp
 *
 *  Created on: 25 paź 2020
 *      Author: kwarc
 */


#include "rcc.hpp"

#include <cmsis/stm32h7xx.h>

#include <hal_system.hpp>

using namespace drivers;

/* TODO Add support for fractional PLL */

//-----------------------------------------------------------------------------

static volatile uint32_t *const bus_map[] =
{
    [static_cast<uint8_t>(rcc::bus::AHB1)] = &RCC->AHB1ENR,
    [static_cast<uint8_t>(rcc::bus::AHB2)] = &RCC->AHB2ENR,
    [static_cast<uint8_t>(rcc::bus::AHB3)] = &RCC->AHB3ENR,
    [static_cast<uint8_t>(rcc::bus::AHB4)] = &RCC->AHB4ENR,
    [static_cast<uint8_t>(rcc::bus::APB1L)] = &RCC->APB1LENR,
    [static_cast<uint8_t>(rcc::bus::APB1H)] = &RCC->APB1HENR,
    [static_cast<uint8_t>(rcc::bus::APB2)] = &RCC->APB2ENR,
    [static_cast<uint8_t>(rcc::bus::APB3)] = &RCC->APB3ENR,
    [static_cast<uint8_t>(rcc::bus::APB4)] = &RCC->APB4ENR,
};

static const uint8_t presc_table[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

//-----------------------------------------------------------------------------

static uint32_t get_ahb_freq(void)
{
    return rcc::get_sysclk_freq() >> ((presc_table[(RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos]) & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb1_freq(void)
{
    return get_ahb_freq() >> ((presc_table[(RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos]) & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb2_freq(void)
{
    return get_ahb_freq() >> ((presc_table[(RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos]) & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb3_freq(void)
{
    return get_ahb_freq() >> ((presc_table[(RCC->D1CFGR & RCC_D1CFGR_D1PPRE) >> RCC_D1CFGR_D1PPRE_Pos]) & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb4_freq(void)
{
    return get_ahb_freq() >> ((presc_table[(RCC->D3CFGR & RCC_D3CFGR_D3PPRE) >> RCC_D3CFGR_D3PPRE_Pos]) & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint16_t get_ahb_presc(void)
{
    return 1 << (presc_table[(RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos] & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb1_presc(void)
{
    return 1 << (presc_table[(RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos] & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb2_presc(void)
{
    return 1 << (presc_table[(RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos] & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb3_presc(void)
{
    return 1 << (presc_table[(RCC->D1CFGR & RCC_D1CFGR_D1PPRE) >> RCC_D1CFGR_D1PPRE_Pos] & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb4_presc(void)
{
    return 1 << (presc_table[(RCC->D3CFGR & RCC_D3CFGR_D3PPRE) >> RCC_D3CFGR_D3PPRE_Pos] & 0x1FU);
}

//-----------------------------------------------------------------------------

static uint32_t get_sysclk_from_pll_source(void)
{
    /* PLL_VCO = (HSE_VALUE, CSI_VALUE or HSI_VALUE/HSIDIV) / PLLM * (PLLN + FRACN), SYSCLK = PLL_VCO / PLLP */
    const uint32_t pll_m = (RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1) >> RCC_PLLCKSELR_DIVM1_Pos;
    if (pll_m == 0)
        return 0;

    uint32_t pll_clk_src;
    if (((uint32_t) (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC)) != RCC_PLLCKSELR_PLLSRC_HSI)
    {
        /* HSE used as PLL clock source */
        pll_clk_src = hal::system::hse_clock;
    }
    else
    {
        /* HSI used as PLL clock source */
        const uint32_t hsi_div = ((RCC-> CR & RCC_CR_HSIDIV) >> RCC_CR_HSIDIV_Pos) + 1;
        pll_clk_src = hal::system::hsi_clock / hsi_div;
    }

    const uint32_t pll_n = ((RCC->PLL1DIVR & RCC_PLL1DIVR_N1) >> RCC_PLL1DIVR_N1_Pos) + 1;
    const uint32_t pll_p = ((RCC->PLL1DIVR & RCC_PLL1DIVR_P1) >> RCC_PLL1DIVR_P1_Pos) + 1;
    const uint32_t pllvco = ((uint64_t) pll_clk_src * ((uint64_t) pll_n) / (uint64_t) pll_m);

    return pllvco / pll_p;
}

//-----------------------------------------------------------------------------

void rcc::reset(void)
{
    /* Set HSION bit */
    RCC->CR |= RCC_CR_HSION;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, CSSON , CSION,RC48ON, CSIKERON PLL1ON, PLL2ON and PLL3ON bits */
    RCC->CR &= 0xEAF6ED7FU;

    /* Reset D1CFGR register */
    RCC->D1CFGR = 0x00000000;

    /* Reset D2CFGR register */
    RCC->D2CFGR = 0x00000000;

    /* Reset D3CFGR register */
    RCC->D3CFGR = 0x00000000;

    /* Reset PLLCKSELR register */
    RCC->PLLCKSELR = 0x00000000;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x00000000;
    /* Reset PLL1DIVR register */
    RCC->PLL1DIVR = 0x00000000;
    /* Reset PLL1FRACR register */
    RCC->PLL1FRACR = 0x00000000;

    /* Reset PLL2DIVR register */
    RCC->PLL2DIVR = 0x00000000;

    /* Reset PLL2FRACR register */

    RCC->PLL2FRACR = 0x00000000;
    /* Reset PLL3DIVR register */
    RCC->PLL3DIVR = 0x00000000;

    /* Reset PLL3FRACR register */
    RCC->PLL3FRACR = 0x00000000;

    /* Reset HSEBYP bit */
    RCC->CR &= 0xFFFBFFFFU;

    /* Disable all interrupts */
    RCC->CIER = 0x00000000;
}

//-----------------------------------------------------------------------------

void rcc::reset_all_periph(void)
{
    /* TODO: Verify this values */

//    RCC->AHB1RSTR = 0xFFFFFFFF;
//    RCC->AHB2RSTR = 0xFFFFFFFF;
//    RCC->AHB3RSTR = 0xFFFFFFFF;
//    RCC->APB1LRSTR = 0xFFFFFFFF;
//    RCC->APB1HRSTR = 0xFFFFFFFF;
//    RCC->APB2RSTR = 0xFFFFFFFF;
//
//    RCC->AHB1RSTR = 0x00000000;
//    RCC->AHB2RSTR = 0x00000000;
//    RCC->AHB3RSTR = 0x00000000;
//    RCC->APB1LRSTR = 0x00000000;
//    RCC->APB1HRSTR = 0x00000000;
//    RCC->APB2RSTR = 0x00000000;
}

//-----------------------------------------------------------------------------

void rcc::disable_all_periph_clocks(void)
{
    /* TODO: Verify this values */

//    RCC->AHB1ENR = 0x00100000;
//    RCC->AHB2ENR = 0x00000000;
//    RCC->AHB3ENR = 0x00000000;
//    RCC->AHB4ENR = 0x00000000;
//    RCC->APB1LENR = 0x00000000;
//    RCC->APB1HENR = 0x00000000;
//    RCC->APB2ENR = 0x00000000;
//
//    RCC->AHB1LPENR = 0x1E03C023;
//    RCC->AHB2LPENR = 0xE0000271;
//    RCC->AHB3LPENR = 0xF0015131;
//    RCC->APB1LLPENR = 0xE8FFCBFF;
//    RCC->APB1HLPENR = 0x00000136;
//    RCC->APB2LPENR = 0x31D73033;
}

//-----------------------------------------------------------------------------

void rcc::enable_periph_clock(const periph_bus &pbus, bool en)
{
    volatile uint32_t *reg = static_cast<volatile uint32_t*>(bus_map[static_cast<uint8_t>(pbus.bus)]);

    if (en)
        *reg |= pbus.periph;
    else
        *reg &= ~(pbus.periph);

    __DSB();
}

//-----------------------------------------------------------------------------

void rcc::set_main_pll(uint32_t src, const pll_cfg &pll, const bus_presc &presc)
{
    /* Enable selected clock source */
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC, src);
    if (src == RCC_PLLCKSELR_PLLSRC_HSE)
        toggle_hse(true);
    else
        toggle_hsi(true);

    /* Select regulator voltage output Scale 1 mode, SMPS: ON, LDO: OFF, system frequency up to 400 MHz */
    MODIFY_REG(PWR->CR3, (PWR_CR3_SMPSLEVEL | PWR_CR3_SMPSEXTHP | PWR_CR3_SMPSEN | PWR_CR3_LDOEN | PWR_CR3_BYPASS), PWR_CR3_SMPSEN);
    MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, PWR_D3CR_VOS_0 | PWR_D3CR_VOS_1);
    while (READ_BIT(PWR->D3CR, PWR_D3CR_VOSRDY) == 0);

    /* Set prescalers for SYSCLK, AHB & APBx buses */
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1CPRE, presc.sys);
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, presc.ahb);
    MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1, presc.apb1);
    MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2, presc.apb2);
    MODIFY_REG(RCC->D3CFGR, RCC_D3CFGR_D3PPRE, presc.apb4);

    /* Configure the main PLL */
    RCC->CR &= ~RCC_CR_PLL1ON;
    RCC->PLLCFGR |= 0b10 << RCC_PLLCFGR_PLL1RGE_Pos; // Input range: 4 - 8 MHz
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM1, pll.m << RCC_PLLCKSELR_DIVM1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_N1, (pll.n-1) << RCC_PLL1DIVR_N1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_P1, (pll.p-1) << RCC_PLL1DIVR_P1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_Q1, (pll.q-1) << RCC_PLL1DIVR_Q1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_R1, (pll.r-1) << RCC_PLL1DIVR_R1_Pos);

    /* Enable the main PLL */
    RCC->CR |= RCC_CR_PLL1ON;

    while ((RCC->CR & RCC_CR_PLL1RDY) == 0)
    {
        /* Wait till the main PLL is ready */
    }

    /* Select the main PLL as system clock source */
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL1);

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1)
    {
        /* Wait till the main PLL is used as system clock source */
    }

    /* Disable unused clock source */
    if (src == RCC_PLLCKSELR_PLLSRC_HSE)
        toggle_hsi(false);
    else
        toggle_hse(false);
}

//-----------------------------------------------------------------------------

void rcc::set_2nd_pll(const pll_cfg &pll)
{
    /* Disable the PLL */
    RCC->CR &= ~RCC_CR_PLL2ON;

    RCC->PLLCFGR |= 0b10 << RCC_PLLCFGR_PLL2RGE_Pos; // Input range: 4 - 8 MHz
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM2, pll.m << RCC_PLLCKSELR_DIVM2_Pos);
    MODIFY_REG(RCC->PLL2DIVR, RCC_PLL2DIVR_N2, (pll.n-1) << RCC_PLL2DIVR_N2_Pos);
    MODIFY_REG(RCC->PLL2DIVR, RCC_PLL2DIVR_P2, (pll.p-1) << RCC_PLL2DIVR_P2_Pos);
    MODIFY_REG(RCC->PLL2DIVR, RCC_PLL2DIVR_Q2, (pll.q-1) << RCC_PLL2DIVR_Q2_Pos);
    MODIFY_REG(RCC->PLL2DIVR, RCC_PLL2DIVR_R2, (pll.r-1) << RCC_PLL2DIVR_R2_Pos);

    /* Enable the PLL */
    RCC->CR |= RCC_CR_PLL2ON;

    while ((RCC->CR & RCC_CR_PLL2RDY) == 0)
    {
        /* Wait till the PLL is ready */
    }
}

//-----------------------------------------------------------------------------

void rcc::set_3rd_pll(const pll_cfg &pll)
{
    /* Disable the PLL */
    RCC->CR &= ~RCC_CR_PLL3ON;

    RCC->PLLCFGR |= 0b10 << RCC_PLLCFGR_PLL3RGE_Pos; // Input range: 4 - 8 MHz
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM3, pll.m << RCC_PLLCKSELR_DIVM3_Pos);
    MODIFY_REG(RCC->PLL3DIVR, RCC_PLL3DIVR_N3, (pll.n-1) << RCC_PLL3DIVR_N3_Pos);
    MODIFY_REG(RCC->PLL3DIVR, RCC_PLL3DIVR_P3, (pll.p-1) << RCC_PLL3DIVR_P3_Pos);
    MODIFY_REG(RCC->PLL3DIVR, RCC_PLL3DIVR_Q3, (pll.q-1) << RCC_PLL3DIVR_Q3_Pos);
    MODIFY_REG(RCC->PLL3DIVR, RCC_PLL3DIVR_R3, (pll.r-1) << RCC_PLL3DIVR_R3_Pos);

    /* Enable the PLL */
    RCC->CR |= RCC_CR_PLL3ON;

    while ((RCC->CR & RCC_CR_PLL3RDY) == 0)
    {
        /* Wait till the PLL is ready */
    }
}

//-----------------------------------------------------------------------------

void rcc::toggle_hsi(bool state)
{
    if (state)
    {
        RCC->CR |= RCC_CR_HSION;
        while ((RCC->CR & RCC_CR_HSIRDY) == 0)
        {
            /* Wait for HSI oscillator stabilization. */
        }
    }
    else
    {
        RCC->CR &= ~RCC_CR_HSION;
    }
}

void rcc::toggle_hse(bool state)
{
    if (state)
    {
        RCC->CR |= RCC_CR_HSEON;
        while ((RCC->CR & RCC_CR_HSERDY) == 0)
        {
            /* Wait for HSE oscillator stabilization. */
        }
    }
    else
    {
        RCC->CR &= ~RCC_CR_HSEON;
    }
}

//-----------------------------------------------------------------------------

void rcc::toggle_lsi(bool state)
{
    if (state)
    {
        RCC->CSR |= RCC_CSR_LSION;
        while ((RCC->CSR & RCC_CSR_LSIRDY) == 0)
        {
            /* Wait for LSI oscillator stabilization. */
        }
    }
    else
    {
        RCC->CSR &= ~RCC_CSR_LSION;
    }
}

//-----------------------------------------------------------------------------

void rcc::toggle_lse(bool state)
{
    /* Enable access to backup domain. */
    PWR->CR1 |= PWR_CR1_DBP;

    if (state)
    {
        RCC->BDCR |= RCC_BDCR_LSEON;
        while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0)
        {
            /* Wait for LSE oscillator stabilization. */
        }
    }
    else
    {
        RCC->BDCR &= ~RCC_BDCR_LSEON;
    }

    /* Disable access to backup domain. */
    PWR->CR1 &= ~PWR_CR1_DBP;
}

//-----------------------------------------------------------------------------

uint32_t rcc::get_sysclk_freq(void)
{
    uint32_t sysclockfreq = 0U;

    /* Get SYSCLK source */
    switch (RCC->CFGR & RCC_CFGR_SWS)
    {
    case RCC_CFGR_SWS_HSI: /* HSI used as system clock source */
        sysclockfreq = hal::system::hsi_clock;
        break;

    case RCC_CFGR_SWS_HSE: /* HSE used as system clock source */
        sysclockfreq = hal::system::hse_clock;
        break;

    case RCC_CFGR_SWS_PLL1: /* PLL1 used as system clock source */
        sysclockfreq = get_sysclk_from_pll_source();
        break;

    default:
        sysclockfreq = hal::system::hsi_clock;
        break;
    }


    uint32_t d1_clk_prescaler = 1;
    const uint32_t d1cpre_reg = (RCC->D1CFGR & RCC_D1CFGR_D1CPRE) >> RCC_D1CFGR_D1CPRE_Pos;
    if (d1cpre_reg != 0)
        d1_clk_prescaler = 1 << (d1cpre_reg - 7);

    return sysclockfreq /= d1_clk_prescaler;
}

//-----------------------------------------------------------------------------

uint32_t rcc::get_bus_freq(rcc::bus bus)
{
    switch (bus)
    {
    case rcc::bus::AHB1:
    case rcc::bus::AHB2:
    case rcc::bus::AHB3:
    case rcc::bus::AHB4:
        return get_ahb_freq();
        break;
    case rcc::bus::APB1L:
    case rcc::bus::APB1H:
        return get_apb1_freq();
        break;
    case rcc::bus::APB2:
        return get_apb2_freq();
        break;
    case rcc::bus::APB3:
        return get_apb3_freq();
        break;
    case rcc::bus::APB4:
        return get_apb4_freq();
        break;
    default:
        return 0;
    }
}

//-----------------------------------------------------------------------------

int16_t rcc::get_bus_presc(rcc::bus bus)
{
    switch (bus)
    {
    case rcc::bus::AHB1:
    case rcc::bus::AHB2:
    case rcc::bus::AHB3:
    case rcc::bus::AHB4:
        return get_ahb_presc();
        break;
    case rcc::bus::APB1L:
    case rcc::bus::APB1H:
        return get_apb1_presc();
        break;
    case rcc::bus::APB2:
        return get_apb2_presc();
        break;
    case rcc::bus::APB3:
        return get_apb3_presc();
        break;
    case rcc::bus::APB4:
        return get_apb4_presc();
        break;
    default:
        return -1;
    }
}

//-----------------------------------------------------------------------------

rcc::reset_source rcc::get_reset_source(void)
{
    return static_cast<rcc::reset_source>(RCC->RSR >> (RCC_RSR_RMVF_Pos + 1));
}

void rcc::clear_reset_source(void)
{
    RCC->RSR |= RCC_RSR_RMVF;
}

//-----------------------------------------------------------------------------
