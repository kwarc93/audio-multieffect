/*
 * hal.hpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_SYSTEM_HPP_
#define HAL_SYSTEM_HPP_

#define HAL_SYSTEM_RTOS_ENABLED

#include <hal_system_impl.hpp>

#ifdef HAL_SYSTEM_RTOS_ENABLED
#include <FreeRTOS.h>
#include <task.h>

namespace hal::system
{
    class critical_section
    {
    public:
        critical_section() : from_isr(xPortIsInsideInterrupt())
        {
            interrupt_mask = from_isr ? taskENTER_CRITICAL_FROM_ISR() : (taskENTER_CRITICAL(), 0);
        }

        ~critical_section()
        {
            from_isr ? taskEXIT_CRITICAL_FROM_ISR(interrupt_mask) : taskEXIT_CRITICAL();
        }

        critical_section(const critical_section&) = delete;
        critical_section& operator=(const critical_section&) = delete;
        critical_section(critical_section&&) = delete;
        critical_section& operator=(critical_section&&) = delete;

    private:
        bool from_isr {};
        UBaseType_t interrupt_mask {};
    };
}
#else
namespace hal::system
{
    using critical_section = drivers::core_critical_section;
}
#endif

#endif /* HAL_SYSTEM_HPP_ */
