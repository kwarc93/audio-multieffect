/*
 * i2c_manager.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef I2C_MANAGER_HPP_
#define I2C_MANAGER_HPP_

#include <variant>

#include <hal/hal_i2c.hpp>

#include "active_object.hpp"

struct i2c_manager_event
{
    struct transfer_evt_t
    {
        enum class result { ok, error };

        std::byte address;
        std::byte *tx_data;
        std::size_t tx_size;
        std::byte *rx_data;
        std::size_t rx_size;
        std::function<void(result res)> callback;
    };

    using holder = std::variant<transfer_evt_t>;
};

class i2c_manager : public i2c_manager_event, public active_object<i2c_manager_event::holder>
{
public:
    i2c_manager(hal::interface::i2c *i2c) : active_object("i2c_manager", osPriorityHigh, 1024), i2c_driver { i2c }
    {

    }

    ~i2c_manager()
    {

    }

    i2c_manager& instance(void)
    {
        static i2c_manager i2c1_manager { hal::i2c::main::get_instance() };
        return i2c1_manager;
    }
private:
    hal::interface::i2c *i2c_driver;

    void dispatch(const event &e) override
    {
        std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
    }

    /* Event handlers */
    void event_handler(const transfer_evt_t &e)
    {

    }
};



#endif /* I2C_MANAGER_HPP_ */
