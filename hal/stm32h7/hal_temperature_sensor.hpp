/*
 * hal_temperature_sensor.hpp
 *
 *  Created on: 25 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_TEMPERATURE_SENSOR_HPP_
#define HAL_TEMPERATURE_SENSOR_HPP_

#include <hal_interface.hpp>

#include <drivers/stm32f7/core.hpp>

namespace hal
{

//-----------------------------------------------------------------------------

    class temperature_sensor
    {
    public:
        temperature_sensor(hal::interface::temperature_sensor *interface);
        virtual ~temperature_sensor(void);

        enum unit { celsius, farenheit, kelvin };
        virtual float read_temperature(enum unit unit = celsius);

    protected:
        hal::interface::temperature_sensor *interface;
    };

//-----------------------------------------------------------------------------

    class internal_temperature_sensor : public temperature_sensor
    {
    public:
        internal_temperature_sensor(void) :
            temperature_sensor(new drivers::core_temperature_sensor()) {}
    };

//-----------------------------------------------------------------------------

}

#endif /* HAL_TEMPERATURE_SENSOR_HPP_ */
