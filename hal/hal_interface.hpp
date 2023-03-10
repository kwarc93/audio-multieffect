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
        virtual ~serial() {};
        virtual std::byte read(void) = 0;
        virtual void write(std::byte byte) = 0;
        virtual std::size_t read(std::byte *data, std::size_t size) = 0;
        virtual std::size_t write(const std::byte *data, std::size_t size) = 0;

        typedef std::function<void(const std::byte *data, std::size_t bytes_read)> read_cb_t;
        typedef std::function<void(std::size_t bytes_written)> write_cb_t;

        virtual void read_async(std::byte *data, std::size_t size, const read_cb_t &callback, bool listen) = 0;
        virtual void write_async(const std::byte *data, std::size_t size, const write_cb_t &callback) = 0;
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
}

#endif /* HAL_INTERFACE_HPP_ */
