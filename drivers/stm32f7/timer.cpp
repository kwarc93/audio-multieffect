/*
 * timer.cpp
 *
 *  Created on: 10 lis 2020
 *      Author: kwarc
 */

#include "timer.hpp"

#include <map>
#include <array>

#include <cmsis/stm32f7xx.h>
#include <cmsis/core_cm7.h>

#include <drivers/stm32f7/gpio.hpp>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

struct timer_ch
{
    bool used = false;
    bool inverted = false;
    gpio::af pin_af;
    gpio::io pin;
};

struct timer::timer_hw
{
    TIM_TypeDef *const reg;
    rcc::periph_bus pbus;
    IRQn_Type irq;
    uint8_t irq_priority;

    uint32_t psc_max;
    uint32_t cnt_max;

    std::array<timer_ch, 4> channels;
};

static const std::map<timer::id, timer::timer_hw> timerx =
{
    { timer::id::timer2,
    timer::timer_hw{ TIM2, RCC_PERIPH_BUS(APB1, TIM2), TIM2_IRQn, 15, UINT16_MAX, UINT16_MAX,
    timer_ch{ true, false, gpio::af::af1, { gpio::port::porta, gpio::pin::pin15 }}}},

    { timer::id::timer3,
    timer::timer_hw{ TIM3, RCC_PERIPH_BUS(APB1, TIM3), TIM3_IRQn, 15, UINT16_MAX, UINT16_MAX,
    timer_ch{ true, false, gpio::af::af2, { gpio::port::portb, gpio::pin::pin4 }}}},

    { timer::id::timer12,
    timer::timer_hw{ TIM12, RCC_PERIPH_BUS(APB1, TIM12), TIM8_BRK_TIM12_IRQn, 15, UINT16_MAX, UINT16_MAX,
    timer_ch{ true, false, gpio::af::af9, { gpio::port::porth, gpio::pin::pin6 }}}},
};

timer::timer(id id) : hw (timerx.at(id))
{
    rcc::enable_periph_clock(this->hw.pbus, true);

    /* Enable auto-reload preload and enable timer (upcounting mode). */
    this->hw.reg->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;

    /* Initialize interrupt */
    NVIC_ClearPendingIRQ(hw.irq);
    NVIC_SetPriority(hw.irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), hw.irq_priority, 0));
    NVIC_EnableIRQ(hw.irq);
}

timer::~timer()
{
    /* Disable timer. */
    this->hw.reg->CR1 &= ~TIM_CR1_CEN;

    /* Disable RCC clock. */
    rcc::enable_periph_clock(this->hw.pbus, false);

    /* Deinitialize GPIO */
    for (auto &ch : this->hw.channels)
    {
        if (ch.used)
            gpio::init(ch.pin, gpio::af::af0, gpio::mode::analog);
    }

    /* Disable interrupt */
    NVIC_DisableIRQ(this->hw.irq);
    NVIC_ClearPendingIRQ(this->hw.irq);
}

bool timer::set_frequency(uint32_t frequency)
{
    uint32_t arr = 1;
    uint32_t psc = 1;

    /* Clocks from APB1 and APB2 buses that are connected to timers are multiplied internally by two if prescaler is greater than 1. */
    uint32_t tim_bus_clock = rcc::get_periph_bus_freq(this->hw.pbus.bus);
    if (rcc::get_periph_bus_presc(this->hw.pbus.bus) > 1)
        tim_bus_clock *=2;

    /* Find smallest prescaler value according to specified frequency to achieve best frequency accuracy.
     * Then calculate auto reload register value and check if it fits into timer resolution. */
    for (psc = 1; psc < this->hw.psc_max; psc *= 2)
    {
        arr = tim_bus_clock / (psc * frequency);
        if (arr < this->hw.cnt_max)
        {
            this->hw.reg->ARR = arr - 1;
            this->hw.reg->PSC = psc - 1;

            /* Re-initialize the counter and generate an update of the registers */
            this->hw.reg->EGR |= TIM_EGR_UG;
            return true;
        }
    }

    return false;
}

bool timer::configure_channel(channel ch, channel_mode mode)
{
    const timer_ch &hw_ch = this->hw.channels[static_cast<uint8_t>(ch)];

    /* Only PWM is supported (TODO: write support for other modes) */
    if (mode != channel_mode::pwm)
        return false;

    if (!hw_ch.used)
        return false;

    gpio::init(hw_ch.pin, hw_ch.pin_af, gpio::mode::af);

    /* Set PWM mode 1, edge aligned with preload enable on selected channel. Invert output polarity if needed. */
       switch (ch)
       {
       case channel::ch1:
           this->hw.reg->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
           if (hw_ch.inverted)
               this->hw.reg->CCER |= TIM_CCER_CC1P;
           break;
       case channel::ch2:
           this->hw.reg->CCMR1 |= TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
           if (hw_ch.inverted)
               this->hw.reg->CCER |= TIM_CCER_CC2P;
           break;
       case channel::ch3:
           this->hw.reg->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;
           if (hw_ch.inverted)
               this->hw.reg->CCER |= TIM_CCER_CC3P;
           break;
       case channel::ch4:
           this->hw.reg->CCMR2 |= TIM_CCMR2_OC4PE | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;
           if (hw_ch.inverted)
               this->hw.reg->CCER |= TIM_CCER_CC4P;
           break;
       default:
           return false;
       }

    return true;
}

//-----------------------------------------------------------------------------

counter::counter(id id, mode mode, uint32_t frequency, void (*irq_update_callback)(void)) :
timer(id)
{
    /* Enable interrupt and set callback. */
    this->hw.reg->DIER |= TIM_DIER_UIE;
    this->irq_update_callback = irq_update_callback;

    this->hw.reg->CR1 &= ~TIM_CR1_DIR_Msk;

    switch (mode)
    {
    case mode::upcounting:
    case mode::downcounting:
        this->hw.reg->CR1 |= (static_cast<uint8_t>(mode) << TIM_CR1_DIR_Pos);
        break;
    case mode::updowncounting:
        /* Not supported yet. */
        return;
    default:
        return;
    }

    this->set_frequency(frequency);
}

uint32_t counter::get_value(void)
{
    return this->hw.reg->CNT;
}

//-----------------------------------------------------------------------------

pwm::pwm(id id, const std::vector<channel> &channels, uint32_t frequency, float duty) :
timer(id)
{
    for (auto &ch : channels)
    {
        this->configure_channel(ch, channel_mode::pwm);
        this->set_duty(ch, duty);
        this->enable(ch);
    }

    this->set_frequency(frequency);
}

void pwm::set_duty(channel ch, float duty)
{
    if (duty < 0.0f)
        duty = 0.0f;

    uint32_t ccr = (duty * this->hw.reg->ARR) / 100ul;

    /* Clip capture/compare register value to fit into timer resolution. */
    if (ccr > this->hw.cnt_max)
        ccr = this->hw.cnt_max;

    /* Set capture/compare register according to specified channel. */
    switch (ch)
    {
    case channel::ch1:
        this->hw.reg->CCR1 = ccr;
        break;
    case channel::ch2:
        this->hw.reg->CCR2 = ccr;
        break;
    case channel::ch3:
        this->hw.reg->CCR3 = ccr;
        break;
    case channel::ch4:
        this->hw.reg->CCR4 = ccr;
        break;
    default:
        return;
    }

    /* Re-initialize the counter and generate an update of the registers. */
    this->hw.reg->EGR |= TIM_EGR_UG;
}

void pwm::enable(channel ch)
{
    /* Enable PWM on specified channel. */
    switch (ch)
    {
    case channel::ch1:
        this->hw.reg->CCER |= TIM_CCER_CC1E;
        break;
    case channel::ch2:
        this->hw.reg->CCER |= TIM_CCER_CC2E;
        break;
    case channel::ch3:
        this->hw.reg->CCER |= TIM_CCER_CC3E;
        break;
    case channel::ch4:
        this->hw.reg->CCER |= TIM_CCER_CC4E;
        break;
    default:
        return;
    }
}

void pwm::disable(channel ch)
{
    /* Disable PWM on specified channel. */
    switch (ch)
    {
    case channel::ch1:
        this->hw.reg->CCER &= ~TIM_CCER_CC1E;
        break;
    case channel::ch2:
        this->hw.reg->CCER &= ~TIM_CCER_CC2E;
        break;
    case channel::ch3:
        this->hw.reg->CCER &= ~TIM_CCER_CC3E;
        break;
    case channel::ch4:
        this->hw.reg->CCER &= ~TIM_CCER_CC4E;
        break;
    default:
        return;
    }
}
