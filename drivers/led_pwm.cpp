/*
 * led_pwm.cpp
 *
 *  Created on: 13 lis 2020
 *      Author: kwarc
 */


#include "led_pwm.hpp"

using namespace drivers;

led_pwm::led_pwm(drivers::pwm &pwm) : pwm {pwm}
{

}

void led_pwm::set(uint8_t brightness)
{
    float duty = (100.0f * brightness) / 255.0f;
    this->pwm.set_duty(timer::channel::ch1, duty);
}
