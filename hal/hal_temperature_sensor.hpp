/*
 * hal_temperature_sensor.hpp
 *
 *  Created on: 25 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_TEMPERATURE_SENSOR_HPP_
#define HAL_TEMPERATURE_SENSOR_HPP_

#include <hal_interface.hpp>

namespace hal
{
    class temperature_sensor
    {
    public:
        temperature_sensor(hal::interface::temperature_sensor *interface)
        {
            this->interface = interface;
        }

        virtual ~temperature_sensor(void)
        {

        }

        enum unit { celsius, farenheit, kelvin };
        virtual float read_temperature(enum unit unit = celsius)
        {
            float temp_in_celsius = this->interface->read_temperature();

            switch (unit)
            {
                case celsius:
                    return temp_in_celsius;
                case farenheit:
                    return 9.0f * temp_in_celsius / 5.0f + 32.0f;
                case kelvin:
                    return temp_in_celsius + 273.15f;
                default:
                    return celsius;
            }
        }

    protected:
        hal::interface::temperature_sensor *interface;
    };
}

#include <hal_temperature_sensor_impl.hpp>

#endif /* HAL_TEMPERATURE_SENSOR_HPP_ */
