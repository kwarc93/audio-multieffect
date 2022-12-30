/*
 * timer.hpp
 *
 *  Created on: 9 lis 2020
 *      Author: kwarc
 */

#ifndef STM32F7_TIMER_HPP_
#define STM32F7_TIMER_HPP_

#include <cstdint>
#include <vector>

namespace drivers
{

//-----------------------------------------------------------------------------

class timer
{
public:

    enum class id
    {
        timer1, timer2, timer3, timer4, timer5, timer6, timer7,
        timer8, timer9, timer10, timer11, timer12, timer13, timer14,
    };

    enum class channel
    {
        ch1, ch2, ch3, ch4
    };

    enum class channel_mode
    {
        ic, oc, pwm, one_pulse
    };

    timer(id id);
    virtual ~timer();

    bool set_frequency(uint32_t frequency);
    bool configure_channel(channel ch_id, channel_mode mode);

    struct timer_hw;
protected:
    const timer_hw &hw;
private:
    static constexpr uint8_t number_of_timers = 14;
};

//-----------------------------------------------------------------------------

class counter : public timer
{
public:

    enum class mode
    {
        upcounting, downcounting, updowncounting
    };

    counter(id id, mode mode, uint32_t frequency, void (*irq_update_callback)(void));
    ~counter() {};

    uint32_t get_value(void);
private:
    void (*irq_update_callback)(void);
};

//-----------------------------------------------------------------------------

class pwm : public timer
{
public:
    pwm(id id, const std::vector<channel> &channels, uint32_t frequency, float duty);
    ~pwm() {};

    void set_duty(channel ch, float duty);
    void enable(channel ch);
    void disable(channel ch);
};

//-----------------------------------------------------------------------------

}

#endif /* STM32F7_TIMER_HPP_ */
