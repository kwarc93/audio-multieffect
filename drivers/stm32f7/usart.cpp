/*
 * usart.cpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#include "usart.hpp"

#include <map>

#include <cmsis/stm32f7xx.h>

#include <hal/hal_system.hpp>

#include <drivers/stm32f7/gpio.hpp>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

struct usart::usart_hw
{
    USART_TypeDef *const reg;
    rcc::periph_bus pbus;

    gpio::af pin_af;
    gpio::io tx_pin;
    gpio::io rx_pin;
};

static const std::map<usart::id, usart::usart_hw> usartx =
{
    {usart::id::usart1,
    {USART1, RCC_PERIPH_BUS(APB2, USART1), gpio::af::af7,
    { gpio::port::porta, gpio::pin::pin9 }, { gpio::port::portb, gpio::pin::pin7 }}},
};

usart::usart(id id, uint32_t baudrate) : hw (usartx.at(id))
{
    rcc::enable_periph_clock(this->hw.pbus, true);

    gpio::init(this->hw.tx_pin, this->hw.pin_af, gpio::mode::af);
    gpio::init(this->hw.rx_pin, this->hw.pin_af, gpio::mode::af);

    this->hw.reg->BRR = (uint32_t) (hal::system::system_clock + baudrate / 2) / baudrate;
    this->hw.reg->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

std::byte usart::read()
{
    while (!( this->hw.reg->ISR & USART_ISR_RXNE));
    return static_cast<std::byte>(this->hw.reg->RDR);
}

void usart::write(std::byte byte)
{
    while (!( this->hw.reg->ISR & USART_ISR_TXE));
    this->hw.reg->TDR = std::to_integer<volatile uint32_t>(byte);
}

std::size_t usart::read(std::byte *data, std::size_t size)
{
    std::size_t bytes_read = 0;

    while (size--)
    {
        *data++ = this->read();
        bytes_read++;
    }

    return bytes_read;
}

std::size_t usart::write(const std::byte *data, std::size_t size)
{
    std::size_t bytes_written = 0;

    while (size--)
    {
        this->write(*data++);
        bytes_written++;
    }

    return bytes_written;
}

