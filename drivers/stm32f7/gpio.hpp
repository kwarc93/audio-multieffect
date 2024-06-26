/*
 * gpio.hpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#ifndef STM32F7_GPIO_HPP_
#define STM32F7_GPIO_HPP_

#include <cstdint>

namespace drivers
{

class gpio final
{
public:
    gpio() = delete;

    enum class port:uint8_t
    {
        porta = 0, portb, portc, portd, porte,
        portf, portg, porth, porti, portj, portk
    };
    enum class pin:uint8_t
    {
        pin0, pin1, pin2, pin3, pin4, pin5, pin6, pin7,
        pin8, pin9, pin10, pin11, pin12, pin13, pin14, pin15,
    };
    enum class mode:uint8_t
    {
        input = 0b00, output = 0b01, af = 0b10, analog = 0b11
    };
    enum class type:uint8_t
    {
        pp = 0b0, od = 0b1
    };
    enum class speed:uint8_t
    {
        low = 0b00, medium = 0b01, high = 0b10, very_high = 0b11
    };
    enum class pupd:uint8_t
    {
        none = 0b00, pu = 0b01, pd = 0b10
    };

    enum class af:uint8_t
    {
        af0 = 0, af1, af2, af3, af4, af5, af6, af7, af8, af9, af10, af11, af12, af13, af14, af15
    };

    struct io
    {
        gpio::port port;
        gpio::pin pin;
    };

    static void configure(const io &io,
                          mode mode = mode::output,
                          af af = af::af0,
                          pupd pupd = pupd::none,
                          type type = type::pp,
                          speed speed = speed::very_high);
    static bool read(const io &io);
    static void write(const io &io, bool state);
    static void toggle(const io &io);
private:
    static void enable_clock(port port);
};

}

#endif /* STM32F7_GPIO_HPP_ */
