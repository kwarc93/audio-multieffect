/*
 * qspi_mt25ql512a.hpp
 *
 *  Created on: 19 lis 2024
 *      Author: kwarc
 */

#ifndef QSPI_MT25QL512A_HPP_
#define QSPI_MT25QL512A_HPP_

#include <hal_interface.hpp>

#include <drivers/stm32.hpp>

namespace drivers
{

class qspi_mt25ql512a : public hal::interface::nvm
{
public:
    qspi_mt25ql512a();
    ~qspi_mt25ql512a();

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
    bool set_4_byte_addresing(bool enabled);
};

}

#endif /* QSPI_MT25QL512A_HPP_ */
