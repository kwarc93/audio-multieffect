/*
 * hal_temperature_sensor_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_TEMPERATURE_SENSOR_IMPL_HPP_
#define STM32F7_HAL_TEMPERATURE_SENSOR_IMPL_HPP_

#include <drivers/stm32f7/core.hpp>

namespace hal::temperature_sensor
{
    class internal_temperature_sensor : public temperature_sensor
    {
    public:
        internal_temperature_sensor(void) : temperature_sensor(&sensor) {}
    private:
        drivers::core_temperature_sensor sensor;
    };
}

#endif /* STM32F7_HAL_TEMPERATURE_SENSOR_IMPL_HPP_ */
