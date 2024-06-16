/*
 * qspi_n25q128a.hpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#ifndef QSPI_N25Q128A_HPP_
#define QSPI_N25Q128A_HPP_

#include <hal/hal_interface.hpp>

namespace drivers
{

class qspi_n25q128a : public hal::interface::nvm
{
public:
    qspi_n25q128a();
    ~qspi_n25q128a();

    bool read(std::byte *data, uint32_t addr, size_t size) override;
    bool write(std::byte *data, uint32_t addr, size_t size) override;
    bool erase_block(uint32_t addr) override;
    bool erase_chip(void) override;
};

}

#endif /* QSPI_N25Q128A_HPP_ */
