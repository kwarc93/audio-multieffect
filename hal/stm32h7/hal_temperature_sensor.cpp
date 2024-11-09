/*
 * hal_temperature_sensor.cpp
 *
 *  Created on: 25 paź 2020
 *      Author: kwarc
 */

#include "hal_temperature_sensor.hpp"

using namespace hal;

temperature_sensor::temperature_sensor(interface::temperature_sensor *interface)
{
    this->interface = interface;
}

temperature_sensor::~temperature_sensor(void)
{
    delete this->interface;
}

float temperature_sensor::read_temperature(enum unit unit)
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
