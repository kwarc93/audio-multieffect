/*
 * i2c_manager.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef I2C_MANAGER_HPP_
#define I2C_MANAGER_HPP_

#include <variant>

#include <hal/hal_interface.hpp>
#include <hal/hal_i2c.hpp>

#include "active_object.hpp"

struct i2c_manager_event
{
    struct transfer_evt_t
    {
        hal::interface::i2c_device::transfer_desc descriptor;
    };

    using holder = std::variant<transfer_evt_t>;
};

class i2c_manager : public i2c_manager_event, public active_object<i2c_manager_event::holder>, public hal::interface::i2c_device
{
public:
    i2c_manager(hal::interface::i2c *drv) : active_object("i2c_manager", osPriorityHigh, 1024), i2c_device(drv)
    {

    }

    ~i2c_manager()
    {

    }

    result transfer(const transfer_desc &descriptor) override
    {
        /* TODO */
        return hal::interface::i2c_device::result::ok;
    }
private:
    void dispatch(const event &e) override
    {
        std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
    }

    /* Event handlers */
    void event_handler(const transfer_evt_t &e)
    {

    }
};

namespace i2c_manager
{

i2c_manager& get_instance(void)
{
    static i2c_manager i2c_main_manager { hal::i2c::main::get_instance() };
    return i2c_main_manager;
}

}



#endif /* I2C_MANAGER_HPP_ */
