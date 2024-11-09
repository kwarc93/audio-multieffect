/*
 * i2c.cpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#include "i2c.hpp"

#include <array>

#include <cmsis/stm32h7xx.h>

#include <drivers/stm32h7/rcc.hpp>
#include <drivers/stm32h7/gpio.hpp>
#include <drivers/stm32h7/i2c_timing_utility.h>


using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

struct i2c::i2c_hw
{
    i2c::id id;
    I2C_TypeDef *const reg;
    rcc::periph_bus pbus;

    gpio::af io_af;
    gpio::io io_sda;
    gpio::io io_scl;
};

static constexpr std::array<i2c::i2c_hw, 1> i2cx
{
    { i2c::id::i2c3, I2C3, RCC_PERIPH_BUS(APB1L, I2C3), gpio::af::af4,
    { gpio::port::porth, gpio::pin::pin8 }, { gpio::port::porth, gpio::pin::pin7 }},
};

//-----------------------------------------------------------------------------
/* public */

i2c::i2c(id hw_id, mode mode, speed speed) :
hw {i2cx.at(static_cast<std::underlying_type_t<id>>(hw_id))}, operating_mode {mode}, bus_speed {speed}
{
    rcc::enable_periph_clock(this->hw.pbus, true);

    gpio::configure(this->hw.io_sda, gpio::mode::af, this->hw.io_af, gpio::pupd::none, gpio::type::od, gpio::speed::low);
    gpio::configure(this->hw.io_scl, gpio::mode::af, this->hw.io_af, gpio::pupd::none, gpio::type::od, gpio::speed::low);

    this->hw.reg->CR1 = 0;
    uint32_t timing = I2C_GetTiming(rcc::get_bus_freq(this->hw.pbus.bus), speed == speed::standard ? 100000 : 400000);
    this->hw.reg->TIMINGR = timing;

    this->hw.reg->CR1 |= I2C_CR1_PE;
}

i2c::~i2c()
{
    this->hw.reg->CR1 &= ~I2C_CR1_PE;
    rcc::enable_periph_clock(this->hw.pbus, false);
    gpio::configure(this->hw.io_sda, gpio::mode::analog);
    gpio::configure(this->hw.io_scl, gpio::mode::analog);
}

void i2c::reset(void)
{
    while (this->hw.reg->ISR & I2C_ISR_BUSY);

    this->hw.reg->CR1 &= ~I2C_CR1_PE;
    while (this->hw.reg->CR1 & I2C_CR1_PE);
    this->hw.reg->CR1 |= I2C_CR1_PE;
}

std::byte i2c::read(void)
{
    this->hw.reg->CR2 = (this->no_stop ? 0 : I2C_CR2_AUTOEND) | 1 << I2C_CR2_NBYTES_Pos | I2C_CR2_START | I2C_CR2_RD_WRN | (this->address << 1);

    while (!(this->hw.reg->ISR & I2C_ISR_RXNE));
    std::byte data = static_cast<std::byte>(this->hw.reg->RXDR);

    if (this->no_stop)
        while (!(this->hw.reg->ISR & I2C_ISR_TC));

    return data;
}

void i2c::write(std::byte byte)
{
    this->hw.reg->CR2 = (this->no_stop ? 0 : I2C_CR2_AUTOEND) | 1 << I2C_CR2_NBYTES_Pos | I2C_CR2_START | (this->address << 1);

    if (this->hw.reg->ISR & I2C_ISR_NACKF)
    {
        this->hw.reg->ICR |= I2C_ICR_NACKCF;
        this->hw.reg->CR2 |= I2C_CR2_STOP;
        this->reset();
        return;
    }

    while (!(this->hw.reg->ISR & I2C_ISR_TXIS));
    this->hw.reg->TXDR = std::to_integer<uint8_t>(byte);

    if (this->no_stop)
        while (!(this->hw.reg->ISR & I2C_ISR_TC));
}

std::size_t i2c::read(std::byte *data, std::size_t size)
{
    if (data == nullptr || size == 0)
        return 0;

    /* Limited to 255 bytes */
    if (size > 255)
        size = 255;

    const std::size_t bytes_to_read = size;

    this->hw.reg->CR2 = (this->no_stop ? 0 : I2C_CR2_AUTOEND) | size << I2C_CR2_NBYTES_Pos | I2C_CR2_START | I2C_CR2_RD_WRN | (this->address << 1);

    while (size)
    {
        if (this->hw.reg->ISR & I2C_ISR_NACKF)
        {
            this->hw.reg->ICR |= I2C_ICR_NACKCF;
            this->hw.reg->CR2 |= I2C_CR2_STOP;
            this->reset();
            return bytes_to_read - size;
        }

        if (this->hw.reg->ISR & I2C_ISR_RXNE)
        {
            *data++ = static_cast<std::byte>(this->hw.reg->RXDR);
            size--;
        }
    }

    if (this->no_stop)
        while (!(this->hw.reg->ISR & I2C_ISR_TC));

    return bytes_to_read - size;
}

std::size_t i2c::write(const std::byte *data, std::size_t size)
{
    if (data == nullptr || size == 0)
        return 0;

    /* Limited to 255 bytes */
    if (size > 255)
        size = 255;

    const std::size_t bytes_to_write = size;

    this->hw.reg->CR2 = (this->no_stop ? 0 : I2C_CR2_AUTOEND) | size << I2C_CR2_NBYTES_Pos | I2C_CR2_START | (this->address << 1);

    while (size)
    {
        if (this->hw.reg->ISR & I2C_ISR_NACKF)
        {
            this->hw.reg->ICR |= I2C_ICR_NACKCF;
            this->hw.reg->CR2 |= I2C_CR2_STOP;
            this->reset();
            return bytes_to_write - size;
        }

        if (this->hw.reg->ISR & I2C_ISR_TXIS)
        {
            this->hw.reg->TXDR = std::to_integer<uint8_t>(*data++);
            size--;
        }
    }

    if (this->no_stop)
        while (!(this->hw.reg->ISR & I2C_ISR_TC));

    return bytes_to_write - size;
}

void i2c::read(std::byte *data, std::size_t size, const read_cb_t &callback)
{
    /* TODO */
    if (callback)
        callback(data, 0);
}

void i2c::write(const std::byte *data, std::size_t size, const write_cb_t &callback)
{
    /* TODO */
    if (callback)
        callback(0);
}


