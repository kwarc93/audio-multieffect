/*
 * usart.cpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#include "usart.hpp"

#include <array>

#include <cmsis/stm32h7xx.h>

#include <hal/hal_system.hpp>

#include <drivers/stm32h7/gpio.hpp>
#include <drivers/stm32h7/rcc.hpp>

using namespace drivers;

struct usart::usart_hw
{
    usart::id id;
    USART_TypeDef *const reg;
    rcc::periph_bus pbus;

    gpio::af io_af;
    gpio::io io_tx;
    gpio::io io_rx;
};

static constexpr std::array<usart::usart_hw, 1> usartx
{
    { usart::id::usart1, USART1, RCC_PERIPH_BUS(APB2, USART1), gpio::af::af7,
    { gpio::port::porta, gpio::pin::pin9 }, { gpio::port::portb, gpio::pin::pin7 }}
};

usart::usart(id hw_id, uint32_t baudrate) :
hw {usartx.at(static_cast<std::underlying_type_t<id>>(hw_id))}
{
    rcc::enable_periph_clock(this->hw.pbus, true);

    gpio::configure(this->hw.io_tx, gpio::mode::af, this->hw.io_af);
    gpio::configure(this->hw.io_rx, gpio::mode::af, this->hw.io_af);

    size_t object_id = static_cast<std::underlying_type_t<id>>(hw_id);
    if (object_id < this->instance.size())
        this->instance[object_id] = this;

    this->hw.reg->BRR = (uint32_t) (rcc::get_bus_freq(this->hw.pbus.bus) + baudrate / 2) / baudrate;
    this->hw.reg->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

usart::~usart()
{
    uint8_t object_id = static_cast<uint8_t>(this->hw.id);
    this->instance[object_id] = nullptr;
}

std::byte usart::read()
{
    while (!(this->hw.reg->ISR & USART_ISR_RXNE_RXFNE));
    return static_cast<std::byte>(this->hw.reg->RDR);
}

void usart::write(std::byte byte)
{
    while (!(this->hw.reg->ISR & USART_ISR_TXE_TXFNF));
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

void usart::read(std::byte *data, std::size_t size, const read_cb_t &callback)
{
    if (size == 0 || data == nullptr)
    {
        this->async_read = {0};
        this->hw.reg->CR1 &= ~USART_CR1_RXNEIE;

        IRQn_Type nvic_irq = static_cast<IRQn_Type>(USART1_IRQn + static_cast<uint8_t>(this->hw.id)); /* TODO: Only supported 1, 2 & 3 */
        NVIC_ClearPendingIRQ(nvic_irq);
        NVIC_DisableIRQ(nvic_irq);

        return;
    }

    this->async_read.counter = 0;
    this->async_read.data = data;
    this->async_read.data_length = this->listening ? 1 : size; /* When listening == true, driver reads one by one forever */
    this->async_read.callback = callback;

    this->hw.reg->CR1 |= USART_CR1_RXNEIE;

    IRQn_Type nvic_irq = static_cast<IRQn_Type>(USART1_IRQn + static_cast<uint8_t>(this->hw.id)); /* TODO: Only supported 1, 2 & 3 */
    NVIC_SetPriority(nvic_irq, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 15, 0 ));
    NVIC_ClearPendingIRQ(nvic_irq);
    NVIC_EnableIRQ(nvic_irq);
}

void usart::write(const std::byte *data, std::size_t size, const write_cb_t &callback)
{
    /* TODO Implement  asynchronous write */
}

void usart::irq_handler(void)
{
    /* Overrun */
    if (this->hw.reg->ISR & USART_ISR_ORE)
    {
        this->hw.reg->ICR |= USART_ICR_ORECF;

        /* TODO: Throw overrun error */
    }

    /* RX not empty */
    if (this->hw.reg->ISR & USART_ISR_RXNE_RXFNE)
    {
        /* Receive the data */
        if (this->async_read.counter < this->async_read.data_length)
        {
            *(this->async_read.data + this->async_read.counter) = static_cast<std::byte>(this->hw.reg->RDR);
            this->async_read.counter++;
        }

        /* Finish reception & call callback */
        if (this->async_read.counter == this->async_read.data_length)
        {
            if (!this->listening)
            {
                this->hw.reg->CR1 &= ~USART_CR1_RXNEIE;
                IRQn_Type nvic_irq = static_cast<IRQn_Type>(USART1_IRQn + static_cast<uint8_t>(this->hw.id)); /* TODO: Only supported 1, 2 & 3 */
                NVIC_DisableIRQ(nvic_irq);
            }

            this->async_read.callback(this->async_read.data, this->async_read.counter);
            this->async_read.counter = 0;
        }

    }
}

