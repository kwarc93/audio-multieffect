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

static const uint8_t ahb_presc_table[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
static const uint8_t apb_presc_table[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

//-----------------------------------------------------------------------------

static uint32_t get_ahb_freq(void)
{
    return (rcc::get_sysclk_freq() >> ahb_presc_table[(RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos]);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb1_freq(void)
{
    return (rcc::get_sysclk_freq() >> apb_presc_table[(RCC->CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos]);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb2_freq(void)
{
    return (rcc::get_sysclk_freq() >> apb_presc_table[(RCC->CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos]);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb3_freq(void)
{
    return (rcc::get_sysclk_freq() >> apb_presc_table[(RCC->CFGR & RCC_D1CFGR_D1PPRE) >> RCC_D1CFGR_D1PPRE_Pos]);
}

//-----------------------------------------------------------------------------

static uint32_t get_apb4_freq(void)
{
    return (rcc::get_sysclk_freq() >> apb_presc_table[(RCC->CFGR & RCC_D3CFGR_D3PPRE) >> RCC_D3CFGR_D3PPRE_Pos]);
}

//-----------------------------------------------------------------------------

static uint8_t get_ahb_presc(void)
{
    return (1 << ahb_presc_table[(RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos]);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb1_presc(void)
{
    return (1 << apb_presc_table[(RCC->CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos]);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb2_presc(void)
{
    return (1 << apb_presc_table[(RCC->CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos]);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb3_presc(void)
{
    return (1 << apb_presc_table[(RCC->CFGR & RCC_D1CFGR_D1PPRE) >> RCC_D1CFGR_D1PPRE_Pos]);
}

//-----------------------------------------------------------------------------

static uint8_t get_apb4_presc(void)
{
    return (1 << apb_presc_table[(RCC->CFGR & RCC_D3CFGR_D3PPRE) >> RCC_D3CFGR_D3PPRE_Pos]);
}

//-----------------------------------------------------------------------------

static uint32_t get_sysclk_from_pll_source(void)
{
    uint32_t pllvco;
    uint32_t pllm;
    uint32_t pllp;

    /* PLL_VCO = (HSE_VALUE, CSI_VALUE or HSI_VALUE/HSIDIV) / PLLM * (PLLN + FRACN), SYSCLK = PLL_VCO / PLLP */
    pllm = RCC->PLLCKSELR & RCC_PLLCKSELR_DIVM1;
    if (((uint32_t) (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC)) != RCC_PLLCKSELR_PLLSRC_HSI)
    {
        /* HSE used as PLL clock source */
        pllvco = (uint32_t) ((((uint64_t) hal::system::hse_clock
               * ((uint64_t) ((RCC->PLL1DIVR & RCC_PLL1DIVR_N1) >> RCC_PLL1DIVR_N1_Pos)))) / (uint64_t) pllm);
    }
    else
    {
        /* HSI used as PLL clock source */
        pllvco = (uint32_t) ((((uint64_t) hal::system::hsi_clock
               * ((uint64_t) ((RCC->PLL1DIVR & RCC_PLL1DIVR_N1) >> RCC_PLL1DIVR_N1_Pos)))) / (uint64_t) pllm);
    }

    pllp = ((((RCC->PLL1DIVR & RCC_PLL1DIVR_P1) >> RCC_PLL1DIVR_P1_Pos) + 1U) * 2U);

    return pllvco / pllp;
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

void rcc::set_main_pll(const main_pll &pll, const bus_presc &presc)
{
    /* Enable selected clock source */
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC, pll.source);
    if (pll.source == RCC_PLLCKSELR_PLLSRC_HSE)
        toggle_hse(true);
    else
        toggle_hsi(true);

    /* Select regulator voltage output Scale 1 mode, SMPS: ON, LDO: OFF, system frequency up to 400 MHz */
    MODIFY_REG(PWR->CR3, (PWR_CR3_SMPSLEVEL | PWR_CR3_SMPSEXTHP | PWR_CR3_SMPSEN | PWR_CR3_LDOEN | PWR_CR3_BYPASS), PWR_CR3_SMPSEN);
    MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, PWR_D3CR_VOS_0 | PWR_D3CR_VOS_1);
    while (READ_BIT(PWR->D3CR, PWR_D3CR_VOSRDY) == 0);

    /* Set prescalers for SYSCLK, AHB & APBx buses */
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1CPRE, RCC_D1CFGR_D1CPRE_DIV1);
    MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, RCC_D1CFGR_HPRE_DIV2);
    MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1, RCC_D2CFGR_D2PPRE1_DIV2);
    MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2, RCC_D2CFGR_D2PPRE2_DIV2);
    MODIFY_REG(RCC->D3CFGR, RCC_D3CFGR_D3PPRE, RCC_D3CFGR_D3PPRE_DIV2);

    /* Configure the main PLL */
    RCC->PLLCFGR |= 0b10 << RCC_PLLCFGR_PLL1RGE_Pos; // Input range: 4 - 8 MHz
    MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM1, pll.m << RCC_PLLCKSELR_DIVM1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_N1, (pll.n-1UL) << RCC_PLL1DIVR_N1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_P1, (pll.p-1UL) << RCC_PLL1DIVR_P1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_Q1, (pll.q-1UL) << RCC_PLL1DIVR_Q1_Pos);
    MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_R1, (pll.r-1UL) << RCC_PLL1DIVR_R1_Pos);

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
    if (pll.source == RCC_PLLCKSELR_PLLSRC_HSE)
        toggle_hsi(false);
    else
        toggle_hse(false);
}

//-----------------------------------------------------------------------------

void rcc::set_sai_pll(const sai_i2s_pll &pll)
{
//    /* Disable the SAI PLL */
//    RCC->CR &= ~RCC_CR_PLLSAION;
//
//    /* Set dividers: after-Q & after-R */
//    RCC->DCKCFGR1 |= (pll.div_q - 1) << RCC_DCKCFGR1_PLLSAIDIVQ_Pos
//                  |  (pll.div_r == 16 ? 0b11 : pll.div_r >> 2) << RCC_DCKCFGR1_PLLSAIDIVR_Pos;
//
//    /* Configure the SAI PLL */
//    RCC->PLLSAICFGR = (pll.n << 6) | (((pll.p >> 1) - 1) << 16) | (pll.q << 24) | (pll.r << 28);
//
//    /* Enable the SAI PLL */
//    RCC->CR |= RCC_CR_PLLSAION;
//
//    while ((RCC->CR & RCC_CR_PLLSAIRDY) == 0)
//    {
//        /* Wait till the SAI PLL is ready */
//    }
}

//-----------------------------------------------------------------------------

void rcc::set_i2s_pll(const sai_i2s_pll &pll)
{
//    /* Disable the I2S PLL */
//    RCC->CR &= ~RCC_CR_PLLI2SON;
//
//    /* Set dividers: after-Q */
//    RCC->DCKCFGR1 |= (pll.div_q - 1) << RCC_DCKCFGR1_PLLI2SDIVQ_Pos;
//
//    /* Configure the I2S PLL */
//    RCC->PLLI2SCFGR = (pll.n << 6) | (((pll.p >> 1) - 1) << 16) | (pll.q << 24) | (pll.r << 28);
//
//    /* Enable the I2S PLL */
//    RCC->CR |= RCC_CR_PLLI2SON;
//
//    while ((RCC->CR & RCC_CR_PLLI2SRDY) == 0)
//    {
//        /* Wait till the I2S PLL is ready */
//    }
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
    return sysclockfreq;
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

int8_t rcc::get_bus_presc(rcc::bus bus)
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
