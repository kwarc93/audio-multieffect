/*
 * gpio.cpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#include "gpio.hpp"

#include <cstdint>
#include <array>

#include <cmsis/stm32f7xx.h>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr std::array<GPIO_TypeDef*, 11> port_map
{{
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF,
    GPIOG, GPIOH, GPIOI, GPIOJ, GPIOK
}};

}

//-----------------------------------------------------------------------------
/* private */

void gpio::enable_clock(port port)
{
    rcc::periph_bus bus;

    switch (port)
    {
        case port::porta:
            bus = RCC_PERIPH_BUS(AHB1, GPIOA);
            break;
        case port::portb:
            bus = RCC_PERIPH_BUS(AHB1, GPIOB);
            break;
        case port::portc:
            bus = RCC_PERIPH_BUS(AHB1, GPIOC);
            break;
        case port::portd:
            bus = RCC_PERIPH_BUS(AHB1, GPIOD);
            break;
        case port::porte:
            bus = RCC_PERIPH_BUS(AHB1, GPIOE);
            break;
        case port::portf:
            bus = RCC_PERIPH_BUS(AHB1, GPIOF);
            break;
        case port::portg:
            bus = RCC_PERIPH_BUS(AHB1, GPIOG);
            break;
        case port::porth:
            bus = RCC_PERIPH_BUS(AHB1, GPIOH);
            break;
        case port::porti:
            bus = RCC_PERIPH_BUS(AHB1, GPIOI);
            break;
        case port::portj:
            bus = RCC_PERIPH_BUS(AHB1, GPIOJ);
            break;
        case port::portk:
            bus = RCC_PERIPH_BUS(AHB1, GPIOK);
            break;
        default:
            return;
    }

    rcc::enable_periph_clock(bus, true);
}

//-----------------------------------------------------------------------------
/* public */

void gpio::configure(const io &io, mode mode, af af, pupd pupd, type type, speed speed)
{
    enable_clock(io.port);

    GPIO_TypeDef *port = port_map[static_cast<uint8_t>(io.port)];
    const uint8_t pin = static_cast<uint8_t>(io.pin);

    port->MODER &= ~(0b11 << (2 * pin));
    port->MODER |= (static_cast<uint8_t>(mode) << (2 * pin));

    port->OTYPER &= ~(0b1 << pin);
    port->OTYPER |= (static_cast<uint8_t>(type) << (pin));

    port->OSPEEDR &= ~(0b11 << (2 * pin));
    port->OSPEEDR |= (static_cast<uint8_t>(speed) << (2 * pin));

    port->PUPDR &= ~(0b11 << (2 * pin));
    port->PUPDR |= (static_cast<uint8_t>(pupd) << (2 * pin));

    const uint8_t afr = pin < 8 ? 0 : 1;
    port->AFR[afr] &= ~(0b1111 << (4 * (pin - afr * 8)));
    port->AFR[afr] |= (static_cast<uint8_t>(af) << (4 * (pin - afr * 8)));
}

bool gpio::read(const io &io)
{
    GPIO_TypeDef *port = port_map[static_cast<uint8_t>(io.port)];
    const uint8_t pin = static_cast<uint8_t>(io.pin);
    return port->IDR & (1 << pin);
}

void gpio::write(const io &io, bool state)
{
    GPIO_TypeDef *port = port_map[static_cast<uint8_t>(io.port)];
    const uint8_t pin = static_cast<uint8_t>(io.pin);
    port->BSRR = state ? (1 << pin) : (1 << pin) << 16;
}

void gpio::toggle(const io &io)
{
    GPIO_TypeDef *port = port_map[static_cast<uint8_t>(io.port)];
    const uint8_t pin = static_cast<uint8_t>(io.pin);
    port->ODR ^= 1 << pin;
}

