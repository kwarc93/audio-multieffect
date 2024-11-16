/*
 * hal_nvm.hpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#ifndef HAL_NVM_HPP_
#define HAL_NVM_HPP_

#include <cstdint>
#include <cstddef>
#include <cassert>

#include <hal_interface.hpp>

namespace hal
{
    class nvm
    {
    public:
        nvm(hal::interface::nvm *interface)
        {
            assert(interface);
            this->interface = interface;
        }

        virtual ~nvm()
        {

        }

        bool read(std::byte *data, uint32_t addr, size_t size)
        {
            return this->interface->read(data, addr, size);
        }

        bool write(std::byte *data, uint32_t addr, size_t size)
        {
            return this->interface->write(data, addr, size);
        }

        bool erase(uint32_t addr, size_t size)
        {
            return this->interface->erase(addr, size);
        }

        bool erase(void)
        {
            return this->interface->erase();
        }

        hal::interface::nvm::status_t status(void)
        {
            return this->interface->status();
        }

        size_t total_size(void) const
        {
            return this->interface->total_size();
        }

        size_t erase_size(void) const
        {
            return this->interface->erase_size();
        }

        size_t prog_size(void) const
        {
            return this->interface->prog_size();
        }

    protected:
        hal::interface::nvm *interface;
    };
}

#include <hal_nvm_impl.hpp>

#endif /* HAL_NVM_HPP_ */
