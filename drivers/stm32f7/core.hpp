/*
 * core.hpp
 *
 *  Created on: 28 paź 2020
 *      Author: kwarc
 */

#ifndef STM32F7_CORE_HPP_
#define STM32F7_CORE_HPP_

#include <hal_interface.hpp>

#include <cmsis/stm32f7xx.h>

namespace drivers
{

class core final
{
public:
    core() = delete;

    static void enable_cycles_counter(void);
    static inline uint32_t get_cycles_counter(void) { return DWT->CYCCNT; }
    static void enter_sleep_mode(void);
    static void enter_stop_mode(void);

    static inline uint32_t &clock = SystemCoreClock;
};

class core_critical_section
{
public:
    core_critical_section(void);
    ~core_critical_section(void);
private:
    uint32_t primask;
};

class core_temperature_sensor : public hal::interface::temperature_sensor
{
public:
    explicit core_temperature_sensor(void);
    float read_temperature(void);
};

}

#endif /* STM32F7_CORE_HPP_ */
