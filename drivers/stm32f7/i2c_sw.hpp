/*
 * i2c_sw.hpp
 *
 *  Created on: 3 sie 2023
 *      Author: kwarc
 */

#ifndef STM32F7_I2C_SW_HPP_
#define STM32F7_I2C_SW_HPP_

#include <hal/hal_interface.hpp>

#include <drivers/stm32f7/gpio.hpp>
#include <drivers/stm32f7/delay.hpp>

#include <cstdint>

namespace drivers
{

class i2c_sw : public hal::interface::i2c
{
public:
    enum class mode { master, slave };
    enum class speed { standard, fast };

    i2c_sw(gpio::io sda, gpio::io scl, mode mode, speed speed);
    std::byte read(void) override;
    void write(std::byte byte) override;
    std::size_t read(std::byte *data, std::size_t size) override;
    std::size_t write(const std::byte *data, std::size_t size) override;
    void read(std::byte *data, std::size_t size, const read_cb_t &callback) override;
    void write(const std::byte *data, std::size_t size, const write_cb_t &callback) override;
private:
    gpio::io sda_io, scl_io;
    mode operating_mode;
    speed bus_speed;

    inline void delay(void)
    {
        delay::us(this->bus_speed == speed::standard ? 5 : 1);
    }


    inline void sda_write(bool state)
    {
        gpio::write(this->sda_io, state);
    }


    inline void scl_write(bool state)
    {
        gpio::write(this->scl_io, state);
    }


    inline bool sda_read(void)
    {
        return gpio::read(this->sda_io);
    }


    inline bool scl_read(void)
    {
        return gpio::read(this->scl_io);
    }


    inline void clock_stretch(void)
    {
        /* Waiting for SCL to go high if the slave is stretching */
        uint32_t timeout_us = 10000;
        while (!scl_read() && timeout_us--) { delay::us(1); };
    }


    inline void send_start(void)
    {
        sda_write(false);
        delay();
        scl_write(false);
        delay();
    }


    inline void send_stop(void)
    {
        sda_write(false);
        delay();
        scl_write(true);
        delay();
        sda_write(true);
        delay();
    }


    inline bool detect_ack(void)
    {
        sda_write(true);
        scl_write(true);
        clock_stretch();
        delay();

        if (sda_read())
            return false;

        scl_write(false);
        delay();

        return true;
    }


    inline void send_ack(bool ack)
    {
        sda_write(ack);

        scl_write(true);
        clock_stretch();
        delay();

        scl_write(false);
        sda_write(true);
        delay();
    }


    inline void write_byte(std::byte byte)
    {
        uint8_t bits = 8;
        do
        {
            sda_write(std::to_integer<bool>(byte & std::byte(1 << 7)));
            scl_write(true);
            clock_stretch();
            delay();

            byte <<= 1;
            bits -= 1;

            scl_write(false);
            delay();
        }
        while (bits > 0);
    }


    inline std::byte read_byte(void)
    {
        uint8_t bits = 8;
        uint8_t byte = 0;
        do
        {
            byte <<= 1;
            scl_write(true);
            clock_stretch();
            delay();

            byte += sda_read();

            bits -= 1;
            scl_write(false);
            delay();
        }
        while (bits > 0);

        return std::byte(byte);
    }


    inline void reset(void)
    {
        for (uint8_t i = 0; i < 9; i++)
        {
            scl_write(false);
            delay();
            scl_write(true);
            delay();
        }

        scl_write(false);
        send_stop();
    }
};

}

#endif /* STM32F7_I2C_SW_HPP_ */
