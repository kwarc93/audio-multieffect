/*
 * qspi_n25q128a.cpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#include "qspi_n25q128a.hpp"

#include <cassert>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

/**
  * @brief  N25Q128A Configuration
  */
#define N25Q128A_FLASH_SIZE                  0x1000000 // 128 Mbits (16MB)
#define N25Q128A_SECTOR_SIZE                 0x10000   // 256 sectors of 64KB
#define N25Q128A_SUBSECTOR_SIZE              0x1000    // 4096 subsectors of 4KB
#define N25Q128A_PAGE_SIZE                   0x100     // 65536 pages of 256B
#define N25Q128A_ADDR_BITS                   (31 - __CLZ(N25Q128A_FLASH_SIZE))

#define N25Q128A_DUMMY_CYCLES_READ           8
#define N25Q128A_DUMMY_CYCLES_READ_QUAD      10

#define N25Q128A_BULK_ERASE_MAX_TIME         250000
#define N25Q128A_SECTOR_ERASE_MAX_TIME       3000
#define N25Q128A_SUBSECTOR_ERASE_MAX_TIME    800
#define N25Q128A_PAGE_PROGRAM_MAX_TIME       5

#define N25Q128A_AUTOPOLLING_INTERVAL        16

/**
  * @brief  N25Q128A Commands
  */
/* Reset Operations */
#define RESET_ENABLE_CMD                     0x66
#define RESET_MEMORY_CMD                     0x99

/* Identification Operations */
#define READ_ID_CMD                          0x9E
#define READ_ID_CMD2                         0x9F
#define MULTIPLE_IO_READ_ID_CMD              0xAF
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD    0x5A

/* Read Operations */
#define READ_CMD                             0x03
#define FAST_READ_CMD                        0x0B
#define DUAL_OUT_FAST_READ_CMD               0x3B
#define DUAL_INOUT_FAST_READ_CMD             0xBB
#define QUAD_OUT_FAST_READ_CMD               0x6B
#define QUAD_INOUT_FAST_READ_CMD             0xEB

/* Write Operations */
#define WRITE_ENABLE_CMD                     0x06
#define WRITE_DISABLE_CMD                    0x04

/* Register Operations */
#define READ_STATUS_REG_CMD                  0x05
#define WRITE_STATUS_REG_CMD                 0x01

#define READ_LOCK_REG_CMD                    0xE8
#define WRITE_LOCK_REG_CMD                   0xE5

#define READ_FLAG_STATUS_REG_CMD             0x70
#define CLEAR_FLAG_STATUS_REG_CMD            0x50

#define READ_NONVOL_CFG_REG_CMD              0xB5
#define WRITE_NONVOL_CFG_REG_CMD             0xB1

#define READ_VOL_CFG_REG_CMD                 0x85
#define WRITE_VOL_CFG_REG_CMD                0x81

#define READ_ENHANCED_VOL_CFG_REG_CMD        0x65
#define WRITE_ENHANCED_VOL_CFG_REG_CMD       0x61

/* Program Operations */
#define PAGE_PROG_CMD                        0x02
#define DUAL_IN_FAST_PROG_CMD                0xA2
#define EXT_DUAL_IN_FAST_PROG_CMD            0xD2
#define QUAD_IN_FAST_PROG_CMD                0x32
#define EXT_QUAD_IN_FAST_PROG_CMD            0x12

/* Erase Operations */
#define SUBSECTOR_ERASE_CMD                  0x20
#define SECTOR_ERASE_CMD                     0xD8
#define BULK_ERASE_CMD                       0xC7

#define PROG_ERASE_RESUME_CMD                0x7A
#define PROG_ERASE_SUSPEND_CMD               0x75

/* One-Time Programmable Operations */
#define READ_OTP_ARRAY_CMD                   0x4B
#define PROG_OTP_ARRAY_CMD                   0x42

/**
  * @brief  N25Q128A Registers
  */
/* Status Register */
#define N25Q128A_SR_WIP                      ((uint8_t)0x01)    /*!< Write in progress */
#define N25Q128A_SR_WREN                     ((uint8_t)0x02)    /*!< Write enable latch */
#define N25Q128A_SR_BLOCKPR                  ((uint8_t)0x5C)    /*!< Block protected against program and erase operations */
#define N25Q128A_SR_PRBOTTOM                 ((uint8_t)0x20)    /*!< Protected memory area defined by BLOCKPR starts from top or bottom */
#define N25Q128A_SR_SRWREN                   ((uint8_t)0x80)    /*!< Status register write enable/disable */

/* Nonvolatile Configuration Register */
#define N25Q128A_NVCR_LOCK                   ((uint16_t)0x0001) /*!< Lock nonvolatile configuration register */
#define N25Q128A_NVCR_DUAL                   ((uint16_t)0x0004) /*!< Dual I/O protocol */
#define N25Q128A_NVCR_QUAB                   ((uint16_t)0x0008) /*!< Quad I/O protocol */
#define N25Q128A_NVCR_RH                     ((uint16_t)0x0010) /*!< Reset/hold */
#define N25Q128A_NVCR_ODS                    ((uint16_t)0x01C0) /*!< Output driver strength */
#define N25Q128A_NVCR_XIP                    ((uint16_t)0x0E00) /*!< XIP mode at power-on reset */
#define N25Q128A_NVCR_NB_DUMMY               ((uint16_t)0xF000) /*!< Number of dummy clock cycles */

/* Volatile Configuration Register */
#define N25Q128A_VCR_WRAP                    ((uint8_t)0x03)    /*!< Wrap */
#define N25Q128A_VCR_XIP                     ((uint8_t)0x08)    /*!< XIP */
#define N25Q128A_VCR_NB_DUMMY                ((uint8_t)0xF0)    /*!< Number of dummy clock cycles */

/* Enhanced Volatile Configuration Register */
#define N25Q128A_EVCR_ODS                    ((uint8_t)0x07)    /*!< Output driver strength */
#define N25Q128A_EVCR_VPPA                   ((uint8_t)0x08)    /*!< Vpp accelerator */
#define N25Q128A_EVCR_RH                     ((uint8_t)0x10)    /*!< Reset/hold */
#define N25Q128A_EVCR_DUAL                   ((uint8_t)0x40)    /*!< Dual I/O protocol */
#define N25Q128A_EVCR_QUAD                   ((uint8_t)0x80)    /*!< Quad I/O protocol */

/* Flag Status Register */
#define N25Q128A_FSR_PRERR                   ((uint8_t)0x02)    /*!< Protection error */
#define N25Q128A_FSR_PGSUS                   ((uint8_t)0x04)    /*!< Program operation suspended */
#define N25Q128A_FSR_VPPERR                  ((uint8_t)0x08)    /*!< Invalid voltage during program or erase */
#define N25Q128A_FSR_PGERR                   ((uint8_t)0x10)    /*!< Program error */
#define N25Q128A_FSR_ERERR                   ((uint8_t)0x20)    /*!< Erase error */
#define N25Q128A_FSR_ERSUS                   ((uint8_t)0x40)    /*!< Erase operation suspended */
#define N25Q128A_FSR_READY                   ((uint8_t)0x80)    /*!< Ready or command in progress */

//-----------------------------------------------------------------------------
/* private */

bool qspi_n25q128a::reset(void)
{
    qspi::command cmd {};

    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = RESET_ENABLE_CMD;

    if (qspi::send(cmd))
    {
        cmd.instruction.value = RESET_MEMORY_CMD;

        if (qspi::send(cmd))
        {
            cmd.mode = qspi::functional_mode::auto_polling;
            cmd.instruction.value = READ_STATUS_REG_CMD;
            cmd.data.mode = qspi::io_mode::single;
            cmd.data.size = 1;
            cmd.auto_polling.match = 0;
            cmd.auto_polling.mask = N25Q128A_SR_WIP;
            cmd.auto_polling.interval = N25Q128A_AUTOPOLLING_INTERVAL;

            return qspi::send(cmd);
        }
    }

    return false;
}

bool qspi_n25q128a::write_enable(void)
{
    qspi::command cmd {};

    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = WRITE_ENABLE_CMD;

    if (qspi::send(cmd))
    {
        cmd.mode = qspi::functional_mode::auto_polling;
        cmd.instruction.value = READ_STATUS_REG_CMD;
        cmd.data.mode = qspi::io_mode::single;
        cmd.data.size = 1;
        cmd.auto_polling.match = N25Q128A_SR_WREN;
        cmd.auto_polling.mask = N25Q128A_SR_WREN;
        cmd.auto_polling.interval = N25Q128A_AUTOPOLLING_INTERVAL;

        return qspi::send(cmd);
    }

    return false;
}

bool qspi_n25q128a::set_dummy_cycles(uint8_t cycles)
{
    uint8_t reg {};
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_read;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = READ_VOL_CFG_REG_CMD;
    cmd.data.mode = qspi::io_mode::single;
    cmd.data.value = reinterpret_cast<std::byte*>(&reg);
    cmd.data.size = 1;

    /* Read volatile configuration register */
    if (!qspi::send(cmd))
        return false;

    if (!this->write_enable())
        return false;

    /* Update volatile configuration register (with new dummy cycles) */
    reg &= ~N25Q128A_VCR_NB_DUMMY;
    reg |= cycles << 4;

    /* Configure the write volatile configuration register command */
    cmd.mode = qspi::functional_mode::indirect_write;
    cmd.instruction.value = WRITE_VOL_CFG_REG_CMD;
    return qspi::send(cmd);
}

//-----------------------------------------------------------------------------
/* public */

qspi_n25q128a::qspi_n25q128a()
{
    /* Initialize QSPI GPIOs */
    static constexpr std::array<const gpio::io, 6> gpios =
    {{
        {gpio::port::portb, gpio::pin::pin6}, // QSPI_NCS
        {gpio::port::portb, gpio::pin::pin2}, // QSPI_CLK
        {gpio::port::portd, gpio::pin::pin11}, // QSPI_D0
        {gpio::port::portd, gpio::pin::pin12}, // QSPI_D1
        {gpio::port::porte, gpio::pin::pin2}, // QSPI_D2
        {gpio::port::portd, gpio::pin::pin13}, // QSPI_D3
    }};

    for (const auto &pin : gpios)
        gpio::configure(pin, gpio::mode::af, gpio::af::af9);
    gpio::configure(gpios.front(), gpio::mode::af, gpio::af::af10);

    /* Configure QSPI peripheral */
    static constexpr qspi::config cfg
    {
        N25Q128A_FLASH_SIZE, // Size: 128Mbit
        6, // min. 50ns
        qspi::clk_mode::mode0,
        2, // AHBCLK / 2 (max. clock: 108 MHz)
        true, // Sample shift
        false, // No DDR mode
        false // No Dual-Flash mode
    };

    qspi::configure(cfg);

    assert(this->reset());
    assert(this->set_dummy_cycles(N25Q128A_DUMMY_CYCLES_READ_QUAD));
}

qspi_n25q128a::~qspi_n25q128a()
{

}

bool qspi_n25q128a::read(std::byte *data, uint32_t addr, size_t size)
{
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_read;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = QUAD_INOUT_FAST_READ_CMD;
    cmd.address.mode = qspi::io_mode::quad;
    cmd.address.value = addr;
    cmd.address.bits = N25Q128A_ADDR_BITS;
    cmd.dummy_cycles = N25Q128A_DUMMY_CYCLES_READ_QUAD;
    cmd.data.mode = qspi::io_mode::quad;
    cmd.data.size = size;
    cmd.data.value = data;

    return qspi::send(cmd, 5000);
}

bool qspi_n25q128a::write(std::byte *data, uint32_t addr, size_t size)
{
    /* Calculate the size between the write address and the end of the page */
    uint32_t current_size = N25Q128A_PAGE_SIZE - (addr % N25Q128A_PAGE_SIZE);

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > size)
        current_size = size;

    uint32_t current_addr = addr;
    const uint32_t end_addr = addr + size;

    qspi::command cmd {};

    cmd.instruction.mode = qspi::io_mode::single;
    cmd.auto_polling.match = 0;
    cmd.auto_polling.mask = N25Q128A_SR_WIP;
    cmd.auto_polling.interval = N25Q128A_AUTOPOLLING_INTERVAL;

    /* Perform the write page by page */
    do
    {
        if (!this->write_enable())
            break;

        cmd.mode = qspi::functional_mode::indirect_write;
        cmd.instruction.value = EXT_QUAD_IN_FAST_PROG_CMD;
        cmd.address.mode = qspi::io_mode::quad;
        cmd.address.value = current_addr;
        cmd.address.bits = N25Q128A_ADDR_BITS;
        cmd.data.mode = qspi::io_mode::quad;
        cmd.data.value = data;
        cmd.data.size = current_size;

        if (!qspi::send(cmd))
            break;

        cmd.mode = qspi::functional_mode::auto_polling;
        cmd.instruction.value = READ_STATUS_REG_CMD;
        cmd.address.mode = qspi::io_mode::none;
        cmd.address.value = 0;
        cmd.address.bits = 0;
        cmd.data.mode = qspi::io_mode::single;
        cmd.data.size = 1;

        if (!qspi::send(cmd, N25Q128A_PAGE_PROGRAM_MAX_TIME))
            break;

        current_addr += current_size;
        data += current_size;
        current_size = ((current_addr + N25Q128A_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : N25Q128A_PAGE_SIZE;
    }
    while (current_addr < end_addr);

    return current_addr == end_addr;
}

bool qspi_n25q128a::erase(uint32_t addr, size_t size)
{
    uint8_t instruction;
    uint32_t timeout;

    if (size == N25Q128A_SECTOR_SIZE)
    {
        instruction = SECTOR_ERASE_CMD;
        timeout = N25Q128A_SECTOR_ERASE_MAX_TIME;
    }
    else if (size == N25Q128A_SUBSECTOR_SIZE)
    {
        instruction = SUBSECTOR_ERASE_CMD;
        timeout = N25Q128A_SUBSECTOR_ERASE_MAX_TIME;
    }
    else
    {
        return false;
    }

    qspi::command cmd {};

    if (this->write_enable())
    {
        cmd.instruction.mode = qspi::io_mode::single;
        cmd.instruction.value = instruction;
        cmd.address.mode = qspi::io_mode::single;
        cmd.address.value = addr;
        cmd.address.bits = N25Q128A_ADDR_BITS;

        if (qspi::send(cmd))
        {
            cmd.mode = qspi::functional_mode::auto_polling;
            cmd.instruction.value = READ_STATUS_REG_CMD;
            cmd.data.mode = qspi::io_mode::single;
            cmd.data.size = 1;
            cmd.auto_polling.match = 0;
            cmd.auto_polling.mask = N25Q128A_SR_WIP;
            cmd.auto_polling.interval = N25Q128A_AUTOPOLLING_INTERVAL;

            return qspi::send(cmd, timeout);
        }
    }

    return false;
}

bool qspi_n25q128a::erase(void)
{
    qspi::command cmd {};

    if (this->write_enable())
    {
        cmd.instruction.mode = qspi::io_mode::single;
        cmd.instruction.value = BULK_ERASE_CMD;

        if (qspi::send(cmd))
        {
            cmd.mode = qspi::functional_mode::auto_polling;
            cmd.instruction.value = READ_STATUS_REG_CMD;
            cmd.data.mode = qspi::io_mode::single;
            cmd.data.size = 1;
            cmd.auto_polling.match = 0;
            cmd.auto_polling.mask = N25Q128A_SR_WIP;
            cmd.auto_polling.interval = N25Q128A_AUTOPOLLING_INTERVAL;

            return qspi::send(cmd, N25Q128A_BULK_ERASE_MAX_TIME);
        }
    }

    return false;
}

qspi_n25q128a::status_t qspi_n25q128a::status(void)
{
    uint8_t reg;
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_read;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = READ_FLAG_STATUS_REG_CMD;
    cmd.data.mode = qspi::io_mode::single;
    cmd.data.value = reinterpret_cast<std::byte*>(&reg);
    cmd.data.size = 1;

    if (!qspi::send(cmd))
        return hal::interface::nvm::status_t::error;

    if (reg & (N25Q128A_FSR_PRERR | N25Q128A_FSR_VPPERR | N25Q128A_FSR_PGERR | N25Q128A_FSR_ERERR))
    {
        return hal::interface::nvm::status_t::error;
    }
    else if (reg & (N25Q128A_FSR_PGSUS | N25Q128A_FSR_ERSUS))
    {
        return hal::interface::nvm::status_t::suspended;
    }
    else if (reg & N25Q128A_FSR_READY)
    {
        return hal::interface::nvm::status_t::ok;
    }
    else
    {
        return hal::interface::nvm::status_t::busy;
    }
}

size_t qspi_n25q128a::total_size(void) const
{
    return N25Q128A_FLASH_SIZE;
}

size_t qspi_n25q128a::erase_size(void) const
{
    return N25Q128A_SUBSECTOR_SIZE;
}

size_t qspi_n25q128a::prog_size(void) const
{
    return N25Q128A_PAGE_SIZE;
}
