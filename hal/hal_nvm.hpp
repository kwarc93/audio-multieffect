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

#include <hal/hal_interface.hpp>

#include <drivers/qspi_n25q128a.hpp>

namespace hal
{

//-----------------------------------------------------------------------------

    class nvm
    {
    public:
        nvm(hal::interface::nvm *interface);
        virtual ~nvm();

        bool read(std::byte *data, uint32_t addr, size_t size);
        bool write(std::byte *data, uint32_t addr, size_t size);
        bool erase(uint32_t addr, size_t size);
        bool erase(void);
        hal::interface::nvm::status_t status(void);
    protected:
        hal::interface::nvm *interface;
    };

//-----------------------------------------------------------------------------

namespace nvms
{
    class qspi_flash : public nvm
    {
    public:
        qspi_flash() : nvm{&drv} {}
    private:
        drivers::qspi_n25q128a drv;
    };

    class internal_flash : public nvm
    {
        // TODO
    };

    class eeprom : public nvm
    {
        // TODO
    };
}

//-----------------------------------------------------------------------------

}

#endif /* HAL_NVM_HPP_ */
