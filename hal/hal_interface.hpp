/*
 * hal_interface.hpp
 *
 *  Created on: 21 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_INTERFACE_HPP_
#define HAL_INTERFACE_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>

namespace hal::interface
{
    class serial
    {
    public:
        typedef std::function<void(const std::byte *data, std::size_t bytes_read)> read_cb_t;
        typedef std::function<void(std::size_t bytes_written)> write_cb_t;

        virtual ~serial() {};
        virtual std::byte read(void) = 0;
        virtual void write(std::byte byte) = 0;
        virtual std::size_t read(std::byte *data, std::size_t size) = 0;
        virtual std::size_t write(const std::byte *data, std::size_t size) = 0;
        virtual void read(std::byte *data, std::size_t size, const read_cb_t &callback, bool listen) = 0;
        virtual void write(const std::byte *data, std::size_t size, const write_cb_t &callback) = 0;
    };

    class i2c
    {
    public:
        typedef std::function<void(const std::byte *data, std::size_t bytes_read)> read_cb_t;
        typedef std::function<void(std::size_t bytes_written)> write_cb_t;

        virtual ~i2c() {};
        virtual std::byte read(uint8_t address) = 0;
        virtual void write(uint8_t address, std::byte byte, bool no_stop) = 0;
        virtual std::size_t read(uint8_t address, std::byte *data, std::size_t size) = 0;
        virtual std::size_t write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop) = 0;
        virtual void read(uint8_t address, std::byte *data, std::size_t size, const read_cb_t &callback) = 0;
        virtual void write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop, const write_cb_t &callback) = 0;
    };

    class i2c_device
    {
    public:

        struct transfer_desc
        {
            uint8_t address;
            const std::byte *tx_data;
            std::size_t tx_size;
            std::byte *rx_data;
            std::size_t rx_size;
            bool error;
        };

        typedef std::function<void(const transfer_desc &transfer)> transfer_cb_t;

        i2c_device(i2c *drv) : driver {drv} {};
        virtual ~i2c_device() { driver = nullptr; };
        virtual void transfer(transfer_desc &descriptor, const transfer_cb_t &callback) = 0;
    protected:
        i2c *driver;
    };

    class temperature_sensor
    {
    public:
        virtual ~temperature_sensor() {};
        virtual float read_temperature(void) = 0;
    };

    class led
    {
    public:
        virtual ~led() {};
        virtual void set(uint8_t brightness) = 0;
        virtual uint8_t get(void) = 0;
    };

    class button
    {
    public:
        virtual ~button() {};
        virtual bool is_pressed(void) = 0;
    };

    template<typename T>
    class glcd
    {
    public:
        using pixel_t = T;

        virtual ~glcd() {};

        virtual uint16_t width(void) = 0;
        virtual uint16_t height(void) = 0;
        virtual uint8_t bpp(void) = 0;

        virtual void draw_pixel(int16_t x, int16_t y, pixel_t pixel) = 0;
        virtual void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data) = 0;

        virtual void enable_vsync(bool state) = 0;
    };
}

#endif /* HAL_INTERFACE_HPP_ */
