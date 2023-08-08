/*
 * i2c_manager.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef I2C_MANAGER_HPP_
#define I2C_MANAGER_HPP_

#include <variant>
#include <vector>
#include <cassert>

#include <hal/hal_interface.hpp>
#include <hal/hal_i2c.hpp>

#include "cmsis_os2.h"

#include "active_object.hpp"

namespace middlewares
{

struct i2c_manager_event
{
    struct schedule_transfer_evt_t
    {
        uint8_t address;
        std::vector<std::byte> tx;
        std::vector<std::byte> rx;
        hal::interface::i2c_device::transfer_cb_t callback;
    };

    struct perform_transfer_evt_t
    {
        hal::interface::i2c_device::transfer_desc &descriptor;
        osThreadId_t caller_thread_id;
    };

    using holder = std::variant<schedule_transfer_evt_t, perform_transfer_evt_t>;
};

class i2c_manager : private i2c_manager_event, private active_object<i2c_manager_event::holder>, public hal::interface::i2c_device
{
public:
    i2c_manager(hal::interface::i2c &drv) : active_object("i2c_manager", osPriorityHigh, 1024), i2c_device(drv)
    {

    }

    ~i2c_manager()
    {

    }

    void transfer(transfer_desc &descriptor) override
    {
        const event e {perform_transfer_evt_t {descriptor, osThreadGetId()}, event::flags::immutable};

        auto bytes_to_write = descriptor.tx_size;
        auto bytes_to_read = descriptor.rx_size;

        descriptor.stat = transfer_desc::status::pending;

        this->send(e);

        bool transfer_done = osThreadFlagsWait(transfer_done_flag, osFlagsWaitAny, osWaitForever) == transfer_done_flag;
        bool transfer_error = !transfer_done || (bytes_to_write != descriptor.tx_size) || (bytes_to_read != descriptor.rx_size);

        descriptor.stat = transfer_error ? transfer_desc::status::error : transfer_desc::status::ok;
    }

    void transfer(const transfer_desc &descriptor, const transfer_cb_t &callback) override
    {
        const event e
        {
            schedule_transfer_evt_t
            {
                descriptor.address,
                std::vector<std::byte>(descriptor.tx_data, descriptor.tx_data + descriptor.tx_size),
                std::vector<std::byte>(descriptor.rx_data, descriptor.rx_data + descriptor.rx_size),
                callback
            }
        };

        this->send(e);
    }
private:

    static constexpr uint32_t transfer_done_flag = 1 << 0;

    void dispatch(const event &e) override
    {
        std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
    }

    void event_handler(const schedule_transfer_evt_t &e)
    {
        this->driver.set_address(e.address);
        this->driver.set_no_stop(e.rx.size() > 0);

        auto bytes_written = this->driver.write(e.tx.data(), e.tx.size());
        this->driver.set_no_stop(false);
        auto bytes_read = this->driver.read(const_cast<std::byte*>(e.rx.data()), e.rx.size());

        if (e.callback)
        {
            const transfer_desc descriptor
            {
                e.address,
                e.tx.data(),
                bytes_written,
                const_cast<std::byte*>(e.rx.data()),
                bytes_read,
                (bytes_written != e.tx.size()) || (bytes_read != e.rx.size()) ?
                transfer_desc::status::error : transfer_desc::status::ok,
            };

            e.callback(descriptor);
        }
    }

    void event_handler(const perform_transfer_evt_t &e)
    {
        this->driver.set_address(e.descriptor.address);
        this->driver.set_no_stop(e.descriptor.rx_size > 0);

        e.descriptor.tx_size = this->driver.write(e.descriptor.tx_data, e.descriptor.tx_size);
        this->driver.set_no_stop(false);
        e.descriptor.rx_size = this->driver.read(const_cast<std::byte*>(e.descriptor.rx_data), e.descriptor.rx_size);

        osThreadFlagsSet(e.caller_thread_id, transfer_done_flag);
    }
};

namespace i2c_managers
{
    namespace main
    {
        inline hal::interface::i2c_device & get_instance(void)
        {
            static i2c_manager i2c_main_manager { hal::i2c::main::get_instance() };
            return i2c_main_manager;
        }
    }
}

}



#endif /* I2C_MANAGER_HPP_ */
