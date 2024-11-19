/*
 * hal_nvm_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_NVM_IMPL_HPP_
#define STM32H7_HAL_NVM_IMPL_HPP_

#include <drivers/qspi_mt25ql512a.hpp>

namespace hal::nvms
{
    class qspi_flash : public nvm
    {
    public:
        qspi_flash() : nvm{&drv} {}
    private:
        drivers::mt25ql512a drv;
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

#endif /* STM32H7_HAL_NVM_IMPL_HPP_ */
