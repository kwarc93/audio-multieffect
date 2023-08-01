/*
 * usart.hpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#ifndef STM32F7_USART_HPP_
#define STM32F7_USART_HPP_

#include <hal/hal_interface.hpp>

#include <cstdint>

namespace drivers
{

class usart : public hal::interface::serial
{
public:
    enum class id
    {
        usart1, usart2, usart3
    };

    usart(id id, uint32_t baudrate);
    ~usart();

    std::byte read(void);
    void write(std::byte byte);
    std::size_t read(std::byte *data, std::size_t size);
    std::size_t write(const std::byte *data, std::size_t size);

    void read(std::byte *data, std::size_t size, const read_cb_t &callback, bool listen = false);
    void write(const std::byte *data, std::size_t size, const write_cb_t &callback);

    void irq_handler(void);

    static inline std::array<usart*, 3> instance; /* Used for global access (e.g. from interrupt) */
    struct usart_hw;
private:
    const usart_hw &hw;

    struct async_read_data
    {
        std::size_t counter;
        std::size_t data_length;
        std::byte *data;
        read_cb_t callback;
        bool listen;
    };

    struct async_write_data
    {
        /* TODO */
        write_cb_t async_write_callback;
    };

    async_read_data async_read;
    async_write_data async_write;
};

}

#endif /* STM32F7_USART_HPP_ */
