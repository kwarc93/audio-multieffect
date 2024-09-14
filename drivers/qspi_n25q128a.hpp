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
    bool erase(uint32_t addr, size_t size = 4096) override;
    bool erase(void) override;

    status_t status(void) override;
    size_t total_size(void) const override;
    size_t erase_size(void) const override;
    size_t prog_size(void) const override;

private:
    bool reset(void);
    bool write_enable(void);
    bool set_dummy_cycles(uint8_t cycles);
};

}

#endif /* QSPI_N25Q128A_HPP_ */
