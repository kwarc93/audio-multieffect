/*
 * exti.hpp
 *
 *  Created on: 11 maj 2024
 *      Author: kwarc
 */

#ifndef STM32F7_EXTI_HPP_
#define STM32F7_EXTI_HPP_

#include <cstdint>
#include <array>
#include <functional>

namespace drivers
{

class exti final
{
public:
    exti() = delete;

    enum class line:uint8_t
    {
        line0 = 0, line1, line2, line3, line4, line5, line6, line7,
        line8, line9, line10, line11, line12, line13, line14, line15,
    };

    enum class mode:uint8_t
    {
        interrupt = 0, event
    };

    enum class edge:uint8_t
    {
        rising = 0, falling, both
    };

    enum class port:uint8_t
    {
        porta = 0, portb, portc, portd, porte,
        portf, portg, porth, porti, portj, portk, none
    };

    using exti_cb_t = std::function<void(void)>;

    static void configure(bool state, line line, port port, mode mode, edge edge, exti_cb_t callback);
    static void trigger(line line);
    static void irq_handler(line line);
    static void irq_handler(line line_start, line line_end);
private:
    static inline std::array<exti_cb_t, 16> callbacks;
};

}

#endif /* STM32F7_EXTI_HPP_ */
