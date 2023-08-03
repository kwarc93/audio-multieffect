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

class i2c_manager_simple : public hal::interface::i2c_device
{
public:
    i2c_manager_simple(hal::interface::i2c *drv) : i2c_device(drv)
    {
        this->mutex = osMutexNew(nullptr);
        assert(this->mutex != nullptr);
    }
    ~i2c_manager_simple()
    {
        osMutexDelete(this->mutex);
        this->mutex = nullptr;
    }

    void transfer(transfer_desc &descriptor, const transfer_cb_t &callback = {}) override
    {
        descriptor.error = true;

        if (osMutexAcquire(this->mutex, osWaitForever) == osOK)
        {
            auto bytes_written = this->driver->write(descriptor.address, descriptor.tx_data, descriptor.tx_size, descriptor.rx_size > 0);
            auto bytes_read = this->driver->read(descriptor.address, descriptor.rx_data, descriptor.rx_size);

            osMutexRelease(this->mutex);

            descriptor.error = (bytes_written != descriptor.tx_size) || (bytes_read != descriptor.rx_size);
            descriptor.tx_size = bytes_written;
            descriptor.rx_size = bytes_read;
        }

        if (callback)
            callback(descriptor);
    }
private:
    osMutexId_t mutex;
};

struct i2c_manager_event
{
    struct transfer_evt_t
    {
        uint8_t address;
        std::vector<std::byte> tx;
        std::vector<std::byte> rx;
        hal::interface::i2c_device::transfer_cb_t callback;
    };

    using holder = std::variant<transfer_evt_t>;
};

class i2c_manager_active : public i2c_manager_event, public active_object<i2c_manager_event::holder>, public hal::interface::i2c_device
{
public:
    i2c_manager_active(hal::interface::i2c *drv) : active_object("i2c_manager", osPriorityHigh, 1024), i2c_device(drv)
    {

    }

    ~i2c_manager_active()
    {

    }

    void transfer(transfer_desc &descriptor, const transfer_cb_t &callback = {}) override
    {
        const event e
        {
            transfer_evt_t
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
    void dispatch(const event &e) override
    {
        std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
    }

    /* Event handlers */
    void event_handler(const transfer_evt_t &e)
    {
        auto bytes_written = this->driver->write(e.address, e.tx.data(), e.tx.size(), e.rx.size() > 0);
        auto bytes_read = this->driver->read(e.address, const_cast<std::byte*>(e.rx.data()), e.rx.size());

        if (e.callback)
        {
            const transfer_desc descriptor
            {
                e.address,
                e.tx.data(),
                bytes_written,
                const_cast<std::byte*>(e.rx.data()),
                bytes_read,
                (bytes_written != e.tx.size()) || (bytes_read != e.rx.size()),
            };

            e.callback(descriptor);
        }
    }
};

namespace i2c_managers
{
    namespace main
    {

        hal::interface::i2c_device & simple(void)
        {
            static i2c_manager_simple i2c_main_manager { &hal::i2c::main::get_instance() };
            return i2c_main_manager;
        }

        hal::interface::i2c_device & active(void)
        {
            static i2c_manager_active i2c_main_manager { &hal::i2c::main::get_instance() };
            return i2c_main_manager;
        }

    }
}

}



#endif /* I2C_MANAGER_HPP_ */
