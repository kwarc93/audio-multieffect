/*
 * hal_nvm_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_NVM_IMPL_HPP_
#define STM32F7_HAL_NVM_IMPL_HPP_

#include <drivers/qspi_n25q128a.hpp>

namespace hal::nvms
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

#endif /* STM32F7_HAL_NVM_IMPL_HPP_ */
