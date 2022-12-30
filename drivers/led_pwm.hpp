/*
 * led_pwm.hpp
 *
 *  Created on: 9 lis 2020
 *      Author: kwarc
 */

#ifndef LED_PWM_HPP_
#define LED_PWM_HPP_

#include <hal/hal_interface.hpp>

#include <drivers/stm32f7/timer.hpp>

namespace drivers
{
    class led_pwm : public hal::interface::led
    {
    public:
        led_pwm(drivers::pwm &pwm);
        void set(uint8_t brightness);

    private:
        drivers::pwm &pwm;
    };
}

#endif /* LED_PWM_HPP_ */
