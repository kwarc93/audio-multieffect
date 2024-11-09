/*
 * i2c_sw.cpp
 *
 *  Created on: 3 sie 2023
 *      Author: kwarc
 */

#include "i2c_sw.hpp"

#include <drivers/stm32h7/delay.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

i2c_sw::i2c_sw(gpio::io sda, gpio::io scl, mode mode, speed speed) :
sda_io {sda}, scl_io {scl}, operating_mode {mode}, bus_speed {speed}
{
    gpio::configure(this->sda_io, gpio::mode::output, gpio::af::af0, gpio::pupd::none, gpio::type::od, gpio::speed::low);
    gpio::configure(this->scl_io, gpio::mode::output, gpio::af::af0, gpio::pupd::none, gpio::type::od, gpio::speed::low);

    gpio::write(this->sda_io, true);
    gpio::write(this->scl_io, true);

    this->reset();
}

std::byte i2c_sw::read(void)
{
    this->send_start();
    this->write_byte(std::byte((this->address << 1) | (1 << 0)));

    std::byte byte { 0 };

    if (!this->detect_ack())
        goto cleanup;

    byte = this->read_byte();
    this->send_ack(true);

cleanup:
     this->send_stop();

     return byte;
}

void i2c_sw::write(std::byte byte)
{
    std::size_t size = 1;

    this->send_start();
    this->write_byte(std::byte(this->address << 1));

    if (!this->detect_ack())
        goto cleanup;

    this->write_byte(byte);

    if (!this->detect_ack())
        goto cleanup;

    size--;

cleanup:
    /* If the user did not request to skip, we send out the stop bit */
    if ((!this->no_stop && size == 0) || (size != 0))
    {
        this->send_stop();
    }
    else
    {
        this->scl_write(true);
        this->sda_write(true);
        this->clock_stretch();
        this->delay();
    }
}

std::size_t i2c_sw::read(std::byte *data, std::size_t size)
{
    if (data == nullptr || size == 0)
        return 0;

    std::size_t bytes_read = 0;

   this->send_start();
   this->write_byte(std::byte((this->address << 1) | (1 << 0)));

   if (!this->detect_ack())
       goto cleanup;

   do
   {
       data[bytes_read++] = this->read_byte();
       this->send_ack(size == 1);
   }
   while (--size);

cleanup:
    this->send_stop();

    return bytes_read;
}

std::size_t i2c_sw::write(const std::byte *data, std::size_t size)
{
    if (data == nullptr || size == 0)
        return 0;

    std::size_t bytes_written = 0;

    this->send_start();
    this->write_byte(std::byte(this->address << 1));

    if (!this->detect_ack())
        goto cleanup;

    do
    {
        this->write_byte(data[bytes_written++]);
        if (!this->detect_ack())
            goto cleanup;
    }
    while (--size);

cleanup:
    /* If the user did not request to skip, we send out the stop bit */
    if ((!this->no_stop && size == 0) || (size != 0))
    {
        this->send_stop();
    }
    else
    {
        this->scl_write(true);
        this->sda_write(true);
        this->clock_stretch();
        this->delay();
    }

    return bytes_written;
}

void i2c_sw::read(std::byte *data, std::size_t size, const read_cb_t &callback)
{
    /* Asynchronous read not possible */
    auto bytes_read = this->read(data, size);
    if (callback)
        callback(data, bytes_read);
}

void i2c_sw::write(const std::byte *data, std::size_t size, const write_cb_t &callback)
{
    /* Asynchronous write not possible */
    auto bytes_writen = this->write(data, size);
    if (callback)
        callback(bytes_writen);
}




