/*
 * qspi_mt25ql512a.cpp
 *
 *  Created on: 19 lis 2024
 *      Author: kwarc
 */

#include "qspi_mt25ql512a.hpp"

#include <cassert>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

/**
  * @brief  MT25QL512A Configuration
  */
#define MT25QL512A_ADDR_BITS                   26
#define MT25QL512A_FLASH_SIZE                  0x4000000 // 512MBit (64MB)
#define MT25QL512A_SECTOR_SIZE                 0x10000   // 1024 sectors of 64KB
#define MT25QL512A_SUBSECTOR_SIZE              0x1000    // 16384 subsectors of 4kB
#define MT25QL512A_PAGE_SIZE                   0x100     // 262144 pages of 256B

#define MT25QL512A_DUMMY_CYCLES_READ           8
#define MT25QL512A_DUMMY_CYCLES_READ_QUAD      8
#define MT25QL512A_DUMMY_CYCLES_READ_DTR       6
#define MT25QL512A_DUMMY_CYCLES_READ_QUAD_DTR  8

#define MT25QL512A_BULK_ERASE_MAX_TIME         460000
#define MT25QL512A_SECTOR_ERASE_MAX_TIME       1000
#define MT25QL512A_SUBSECTOR_ERASE_MAX_TIME    400
#define MT25QL512A_PAGE_PROGRAM_MAX_TIME       2

#define MT25QL512A_AUTOPOLLING_INTERVAL        16

/**
  * @brief  MT25QL512A Commands
  */
/* Reset Operations */
#define MT25QL512A_RESET_ENABLE_CMD                     0x66
#define MT25QL512A_RESET_MEMORY_CMD                     0x99

/* Identification Operations */
#define MT25QL512A_READ_ID_CMD                          0x9E
#define MT25QL512A_READ_ID_CMD2                         0x9F
#define MT25QL512A_MULTIPLE_IO_READ_ID_CMD              0xAF
#define MT25QL512A_READ_SERIAL_FLASH_DISCO_PARAM_CMD    0x5A

/* Read Operations */
#define MT25QL512A_READ_CMD                             0x03
#define MT25QL512A_READ_4_BYTE_ADDR_CMD                 0x13

#define MT25QL512A_FAST_READ_CMD                        0x0B
#define MT25QL512A_FAST_READ_DTR_CMD                    0x0D
#define MT25QL512A_FAST_READ_4_BYTE_ADDR_CMD            0x0C
#define MT25QL512A_FAST_READ_4_BYTE_DTR_CMD             0x0E

#define MT25QL512A_DUAL_OUT_FAST_READ_CMD               0x3B
#define MT25QL512A_DUAL_OUT_FAST_READ_DTR_CMD           0x3D
#define MT25QL512A_DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x3C

#define MT25QL512A_DUAL_INOUT_FAST_READ_CMD             0xBB
#define MT25QL512A_DUAL_INOUT_FAST_READ_DTR_CMD         0xBD
#define MT25QL512A_DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xBC

#define MT25QL512A_QUAD_OUT_FAST_READ_CMD               0x6B
#define MT25QL512A_QUAD_OUT_FAST_READ_DTR_CMD           0x6D
#define MT25QL512A_QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD   0x6C

#define MT25QL512A_QUAD_INOUT_FAST_READ_CMD             0xEB
#define MT25QL512A_QUAD_INOUT_FAST_READ_DTR_CMD         0xED
#define MT25QL512A_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD 0xEC
#define MT25QL512A_QUAD_INOUT_FAST_READ_4_BYTE_DTR_CMD  0xEE

/* Write Operations */
#define MT25QL512A_WRITE_ENABLE_CMD                     0x06
#define MT25QL512A_WRITE_DISABLE_CMD                    0x04

/* Register Operations */
#define MT25QL512A_READ_STATUS_REG_CMD                  0x05
#define MT25QL512A_WRITE_STATUS_REG_CMD                 0x01

#define MT25QL512A_READ_LOCK_REG_CMD                    0xE8
#define MT25QL512A_WRITE_LOCK_REG_CMD                   0xE5

#define MT25QL512A_READ_FLAG_STATUS_REG_CMD             0x70
#define MT25QL512A_CLEAR_FLAG_STATUS_REG_CMD            0x50

#define MT25QL512A_READ_NONVOL_CFG_REG_CMD              0xB5
#define MT25QL512A_WRITE_NONVOL_CFG_REG_CMD             0xB1

#define MT25QL512A_READ_VOL_CFG_REG_CMD                 0x85
#define MT25QL512A_WRITE_VOL_CFG_REG_CMD                0x81

#define MT25QL512A_READ_ENHANCED_VOL_CFG_REG_CMD        0x65
#define MT25QL512A_WRITE_ENHANCED_VOL_CFG_REG_CMD       0x61

#define MT25QL512A_READ_EXT_ADDR_REG_CMD                0xC8
#define MT25QL512A_WRITE_EXT_ADDR_REG_CMD               0xC5

/* Program Operations */
#define MT25QL512A_PAGE_PROG_CMD                        0x02
#define MT25QL512A_PAGE_PROG_4_BYTE_ADDR_CMD            0x12

#define MT25QL512A_DUAL_IN_FAST_PROG_CMD                0xA2
#define MT25QL512A_EXT_DUAL_IN_FAST_PROG_CMD            0xD2

#define MT25QL512A_QUAD_IN_FAST_PROG_CMD                0x32
#define MT25QL512A_EXT_QUAD_IN_FAST_PROG_CMD            0x38
#define MT25QL512A_QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD    0x34

/* Erase Operations */
#define MT25QL512A_SUBSECTOR_ERASE_CMD_4K               0x20
#define MT25QL512A_SUBSECTOR_ERASE_4_BYTE_ADDR_CMD_4K   0x21

#define MT25QL512A_SUBSECTOR_ERASE_CMD_32K              0x52

#define MT25QL512A_SECTOR_ERASE_CMD                     0xD8
#define MT25QL512A_SECTOR_ERASE_4_BYTE_ADDR_CMD         0xDC

#define MT25QL512A_BULK_ERASE_CMD                       0xC7

#define MT25QL512A_PROG_ERASE_RESUME_CMD                0x7A
#define MT25QL512A_PROG_ERASE_SUSPEND_CMD               0x75

/* One-Time Programmable Operations */
#define MT25QL512A_READ_OTP_ARRAY_CMD                   0x4B
#define MT25QL512A_PROG_OTP_ARRAY_CMD                   0x42

/* 4-byte Address Mode Operations */
#define MT25QL512A_ENTER_4_BYTE_ADDR_MODE_CMD           0xB7
#define MT25QL512A_EXIT_4_BYTE_ADDR_MODE_CMD            0xE9

/* Quad Operations */
#define MT25QL512A_ENTER_QUAD_CMD                       0x35
#define MT25QL512A_EXIT_QUAD_CMD                        0xF5
#define MT25QL512A_ENTER_DEEP_POWER_DOWN                0xB9
#define MT25QL512A_RELEASE_FROM_DEEP_POWER_DOWN         0xAB

/* ADVANCED SECTOR PROTECTION Operations*/
#define MT25QL512A_READ_SECTOR_PROTECTION_CMD           0x2D
#define MT25QL512A_PROGRAM_SECTOR_PROTECTION            0x2C
#define MT25QL512A_READ_PASSWORD_CMD                    0x27
#define MT25QL512A_WRITE_PASSWORD_CMD                   0x28
#define MT25QL512A_UNLOCK_PASSWORD_CMD                  0x29
#define MT25QL512A_READ_GLOBAL_FREEZE_BIT               0xA7
#define MT25QL512A_READ_VOLATILE_LOCK_BITS              0xE8
#define MT25QL512A_WRITE_VOLATILE_LOCK_BITS             0xE5

/* ADVANCED SECTOR PROTECTION Operations with 4-Byte Address*/
#define MT25QL512A_WRITE_4_BYTE_VOLATILE_LOCK_BITS      0xE1
#define MT25QL512A_READ_4_BYTE_VOLATILE_LOCK_BITS       0xE0

/* One Time Programmable Operations */
#define MT25QL512A_READ_OTP_ARRAY                       0x4B
#define MT25QL512A_PROGRAM_OTP_ARRAY                    0x42


/**
  * @brief  MT25QL512A Registers
  */
/* Status Register */
#define MT25QL512A_SR_WIP                      ((uint8_t)0x01)    /*!< Write in progress */
#define MT25QL512A_SR_WREN                     ((uint8_t)0x02)    /*!< Write enable latch */
#define MT25QL512A_SR_BLOCKPR                  ((uint8_t)0x5C)    /*!< Block protected against program and erase operations */
#define MT25QL512A_SR_PRBOTTOM                 ((uint8_t)0x20)    /*!< Protected memory area defined by BLOCKPR starts from top or bottom */
#define MT25QL512A_SR_SRWREN                   ((uint8_t)0x80)    /*!< Status register write enable/disable */

/* Non volatile Configuration Register */
#define MT25QL512A_NVCR_NBADDR                 ((uint16_t)0x0001) /*!< 3-bytes or 4-bytes addressing */
#define MT25QL512A_NVCR_SEGMENT                ((uint16_t)0x0002) /*!< Upper or lower 128Mb segment selected by default */
#define MT25QL512A_NVCR_DUAL                   ((uint16_t)0x0004) /*!< Dual I/O protocol */
#define MT25QL512A_NVCR_QUAB                   ((uint16_t)0x0008) /*!< Quad I/O protocol */
#define MT25QL512A_NVCR_RH                     ((uint16_t)0x0010) /*!< Reset/hold */
#define MT25QL512A_NVCR_DTRP                   ((uint16_t)0x0020) /*!< Double transfer rate protocol */
#define MT25QL512A_NVCR_ODS                    ((uint16_t)0x01C0) /*!< Output driver strength */
#define MT25QL512A_NVCR_XIP                    ((uint16_t)0x0E00) /*!< XIP mode at power-on reset */
#define MT25QL512A_NVCR_NB_DUMMY               ((uint16_t)0xF000) /*!< Number of dummy clock cycles */

/* Volatile Configuration Register */
#define MT25QL512A_VCR_WRAP                    ((uint8_t)0x03)    /*!< Wrap */
#define MT25QL512A_VCR_XIP                     ((uint8_t)0x08)    /*!< XIP */
#define MT25QL512A_VCR_NB_DUMMY                ((uint8_t)0xF0)    /*!< Number of dummy clock cycles */

/* Extended Address Register */
#define MT25QL512A_EAR_HIGHEST_SE              ((uint8_t)0x03)    /*!< Select the Highest 128Mb segment */
#define MT25QL512A_EAR_THIRD_SEG               ((uint8_t)0x02)    /*!< Select the Third 128Mb segment */
#define MT25QL512A_EAR_SECOND_SEG              ((uint8_t)0x01)    /*!< Select the Second 128Mb segment */
#define MT25QL512A_EAR_LOWEST_SEG              ((uint8_t)0x00)    /*!< Select the Lowest 128Mb segment (default) */

/* Enhanced Volatile Configuration Register */
#define MT25QL512A_EVCR_ODS                    ((uint8_t)0x07)    /*!< Output driver strength */
#define MT25QL512A_EVCR_RH                     ((uint8_t)0x10)    /*!< Reset/hold */
#define MT25QL512A_EVCR_DTRP                   ((uint8_t)0x20)    /*!< Double transfer rate protocol */
#define MT25QL512A_EVCR_DUAL                   ((uint8_t)0x40)    /*!< Dual I/O protocol */
#define MT25QL512A_EVCR_QUAD                   ((uint8_t)0x80)    /*!< Quad I/O protocol */

/* Flag Status Register */
#define MT25QL512A_FSR_NBADDR                  ((uint8_t)0x01)    /*!< 3-bytes or 4-bytes addressing */
#define MT25QL512A_FSR_PRERR                   ((uint8_t)0x02)    /*!< Protection error */
#define MT25QL512A_FSR_PGSUS                   ((uint8_t)0x04)    /*!< Program operation suspended */
#define MT25QL512A_FSR_PGERR                   ((uint8_t)0x10)    /*!< Program error */
#define MT25QL512A_FSR_ERERR                   ((uint8_t)0x20)    /*!< Erase error */
#define MT25QL512A_FSR_ERSUS                   ((uint8_t)0x40)    /*!< Erase operation suspended */
#define MT25QL512A_FSR_READY                   ((uint8_t)0x80)    /*!< Ready or command in progress */


//-----------------------------------------------------------------------------
/* private */

bool qspi_mt25ql512a::reset(void)
{
    qspi::command cmd {};

    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = MT25QL512A_RESET_ENABLE_CMD;

    if (qspi::send(cmd))
    {
        cmd.instruction.value = MT25QL512A_RESET_MEMORY_CMD;

        if (qspi::send(cmd))
        {
            cmd.mode = qspi::functional_mode::auto_polling;
            cmd.instruction.value = MT25QL512A_READ_STATUS_REG_CMD;
            cmd.data.mode = qspi::io_mode::single;
            cmd.data.size = 1;
            cmd.auto_polling.match = 0;
            cmd.auto_polling.mask = MT25QL512A_SR_WIP;
            cmd.auto_polling.interval = MT25QL512A_AUTOPOLLING_INTERVAL;

            return qspi::send(cmd);
        }
    }

    return false;
}

bool qspi_mt25ql512a::write_enable(void)
{
    qspi::command cmd {};

    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = MT25QL512A_WRITE_ENABLE_CMD;

    if (qspi::send(cmd))
    {
        cmd.mode = qspi::functional_mode::auto_polling;
        cmd.instruction.value = MT25QL512A_READ_STATUS_REG_CMD;
        cmd.data.mode = qspi::io_mode::single;
        cmd.data.size = 1;
        cmd.auto_polling.match = MT25QL512A_SR_WREN;
        cmd.auto_polling.mask = MT25QL512A_SR_WREN;
        cmd.auto_polling.interval = MT25QL512A_AUTOPOLLING_INTERVAL;

        return qspi::send(cmd);
    }

    return false;
}

bool qspi_mt25ql512a::set_dummy_cycles(uint8_t cycles)
{
    uint8_t reg {};
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_read;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = MT25QL512A_READ_VOL_CFG_REG_CMD;
    cmd.data.mode = qspi::io_mode::single;
    cmd.data.value = reinterpret_cast<std::byte*>(&reg);
    cmd.data.size = 1;

    /* Read volatile configuration register */
    if (!qspi::send(cmd))
        return false;

    if (!this->write_enable())
        return false;

    /* Update volatile configuration register (with new dummy cycles) */
    reg &= ~MT25QL512A_VCR_NB_DUMMY;
    reg |= cycles << 4;

    /* Configure the write volatile configuration register command */
    cmd.mode = qspi::functional_mode::indirect_write;
    cmd.instruction.value = MT25QL512A_WRITE_VOL_CFG_REG_CMD;
    return qspi::send(cmd);
}

bool qspi_mt25ql512a::set_4_byte_addresing(bool enabled)
{
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_write;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = enabled ? MT25QL512A_ENTER_4_BYTE_ADDR_MODE_CMD :
                                      MT25QL512A_EXIT_4_BYTE_ADDR_MODE_CMD;

    return qspi::send(cmd);
}

bool qspi_mt25ql512a::set_quad_io_mode(bool enabled)
{
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_write;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = enabled ? MT25QL512A_ENTER_QUAD_CMD :
                                      MT25QL512A_EXIT_QUAD_CMD;

    return qspi::send(cmd);
}

//-----------------------------------------------------------------------------
/* public */

qspi_mt25ql512a::qspi_mt25ql512a()
{
    /* Initialize QSPI GPIOs (Bank 1) */
    static constexpr std::array<const gpio::io, 6> gpios =
    {{
        {gpio::port::portg, gpio::pin::pin6}, // QSPI_NCS
        {gpio::port::portf, gpio::pin::pin10}, // QSPI_CLK
        {gpio::port::portd, gpio::pin::pin11}, // QSPI_D0
        {gpio::port::portf, gpio::pin::pin9}, // QSPI_D1
        {gpio::port::portf, gpio::pin::pin7}, // QSPI_D2
        {gpio::port::portf, gpio::pin::pin6}, // QSPI_D3
    }};

    for (const auto &pin : gpios)
        gpio::configure(pin, gpio::mode::af, gpio::af::af9);
    gpio::configure(gpios[0], gpio::mode::af, gpio::af::af10);
    gpio::configure(gpios[3], gpio::mode::af, gpio::af::af10);

    /* Configure QSPI peripheral */
    static constexpr qspi::config cfg
    {
        512, // Size: 512Mbit
        6, // min. 50ns
        qspi::clk_mode::mode0,
        2, // AHBCLK / 2 (max. clock: 133MHz/90MHz, SDR/DDR)
        true, // Sample shift
        false, // No DDR mode
        false // No Dual-Flash mode
    };

    qspi::configure(cfg);

    assert(this->reset());
    assert(this->set_4_byte_addresing(true));
    assert(this->set_dummy_cycles(MT25QL512A_DUMMY_CYCLES_READ_QUAD));
}

qspi_mt25ql512a::~qspi_mt25ql512a()
{

}

bool qspi_mt25ql512a::read(std::byte *data, uint32_t addr, size_t size)
{
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_read;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = MT25QL512A_QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    cmd.address.mode = qspi::io_mode::quad;
    cmd.address.value = addr;
    cmd.address.bits = MT25QL512A_ADDR_BITS;
    cmd.dummy_cycles = MT25QL512A_DUMMY_CYCLES_READ_QUAD;
    cmd.data.mode = qspi::io_mode::quad;
    cmd.data.size = size;
    cmd.data.value = data;

    return qspi::send(cmd, 5000);
}

bool qspi_mt25ql512a::write(std::byte *data, uint32_t addr, size_t size)
{
    /* Calculate the size between the write address and the end of the page */
    uint32_t current_size = MT25QL512A_PAGE_SIZE - (addr % MT25QL512A_PAGE_SIZE);

    /* Check if the size of the data is less than the remaining place in the page */
    if (current_size > size)
        current_size = size;

    uint32_t current_addr = addr;
    const uint32_t end_addr = addr + size;

    qspi::command cmd {};

    cmd.instruction.mode = qspi::io_mode::single;
    cmd.auto_polling.match = 0;
    cmd.auto_polling.mask = MT25QL512A_SR_WIP;
    cmd.auto_polling.interval = MT25QL512A_AUTOPOLLING_INTERVAL;

    /* Perform the write page by page */
    do
    {
        if (!this->write_enable())
            break;

        cmd.mode = qspi::functional_mode::indirect_write;
        cmd.instruction.value = MT25QL512A_EXT_QUAD_IN_FAST_PROG_CMD;
        cmd.address.mode = qspi::io_mode::quad;
        cmd.address.value = current_addr;
        cmd.address.bits = MT25QL512A_ADDR_BITS;
        cmd.data.mode = qspi::io_mode::quad;
        cmd.data.value = data;
        cmd.data.size = current_size;

        if (!qspi::send(cmd))
            break;

        cmd.mode = qspi::functional_mode::auto_polling;
        cmd.instruction.value = MT25QL512A_READ_STATUS_REG_CMD;
        cmd.address.mode = qspi::io_mode::none;
        cmd.address.value = 0;
        cmd.address.bits = 0;
        cmd.data.mode = qspi::io_mode::single;
        cmd.data.size = 1;

        if (!qspi::send(cmd, MT25QL512A_PAGE_PROGRAM_MAX_TIME))
            break;

        current_addr += current_size;
        data += current_size;
        current_size = ((current_addr + MT25QL512A_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MT25QL512A_PAGE_SIZE;
    }
    while (current_addr < end_addr);

    return current_addr == end_addr;
}

bool qspi_mt25ql512a::erase(uint32_t addr, size_t size)
{
    uint8_t instruction;
    uint32_t timeout;

    if (size == MT25QL512A_SECTOR_SIZE)
    {
        instruction = MT25QL512A_SECTOR_ERASE_4_BYTE_ADDR_CMD;
        timeout = MT25QL512A_SECTOR_ERASE_MAX_TIME;
    }
    else if (size == MT25QL512A_SUBSECTOR_SIZE)
    {
        instruction = MT25QL512A_SUBSECTOR_ERASE_4_BYTE_ADDR_CMD_4K;
        timeout = MT25QL512A_SUBSECTOR_ERASE_MAX_TIME;
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
        cmd.address.mode = qspi::io_mode::quad;
        cmd.address.value = addr;
        cmd.address.bits = MT25QL512A_ADDR_BITS;

        if (qspi::send(cmd))
        {
            cmd.mode = qspi::functional_mode::auto_polling;
            cmd.instruction.value = MT25QL512A_READ_STATUS_REG_CMD;
            cmd.data.mode = qspi::io_mode::single;
            cmd.data.size = 1;
            cmd.auto_polling.match = 0;
            cmd.auto_polling.mask = MT25QL512A_SR_WIP;
            cmd.auto_polling.interval = MT25QL512A_AUTOPOLLING_INTERVAL;

            return qspi::send(cmd, timeout);
        }
    }

    return false;
}

bool qspi_mt25ql512a::erase(void)
{
    qspi::command cmd {};

    if (this->write_enable())
    {
        cmd.instruction.mode = qspi::io_mode::single;
        cmd.instruction.value = MT25QL512A_BULK_ERASE_CMD;

        if (qspi::send(cmd))
        {
            cmd.mode = qspi::functional_mode::auto_polling;
            cmd.instruction.value = MT25QL512A_READ_STATUS_REG_CMD;
            cmd.data.mode = qspi::io_mode::single;
            cmd.data.size = 1;
            cmd.auto_polling.match = 0;
            cmd.auto_polling.mask = MT25QL512A_SR_WIP;
            cmd.auto_polling.interval = MT25QL512A_AUTOPOLLING_INTERVAL;

            return qspi::send(cmd, MT25QL512A_BULK_ERASE_MAX_TIME);
        }
    }

    return false;
}

qspi_mt25ql512a::status_t qspi_mt25ql512a::status(void)
{
    uint8_t reg;
    qspi::command cmd {};

    cmd.mode = qspi::functional_mode::indirect_read;
    cmd.instruction.mode = qspi::io_mode::single;
    cmd.instruction.value = MT25QL512A_READ_FLAG_STATUS_REG_CMD;
    cmd.data.mode = qspi::io_mode::single;
    cmd.data.value = reinterpret_cast<std::byte*>(&reg);
    cmd.data.size = 1;

    if (!qspi::send(cmd))
        return hal::interface::nvm::status_t::error;

    if (reg & (MT25QL512A_FSR_PRERR | MT25QL512A_FSR_PGERR | MT25QL512A_FSR_ERERR))
    {
        return hal::interface::nvm::status_t::error;
    }
    else if (reg & (MT25QL512A_FSR_PGSUS | MT25QL512A_FSR_ERSUS))
    {
        return hal::interface::nvm::status_t::suspended;
    }
    else if (reg & MT25QL512A_FSR_READY)
    {
        return hal::interface::nvm::status_t::ok;
    }
    else
    {
        return hal::interface::nvm::status_t::busy;
    }
}

size_t qspi_mt25ql512a::total_size(void) const
{
    return MT25QL512A_FLASH_SIZE;
}

size_t qspi_mt25ql512a::erase_size(void) const
{
    return MT25QL512A_SUBSECTOR_SIZE;
}

size_t qspi_mt25ql512a::prog_size(void) const
{
    return MT25QL512A_PAGE_SIZE;
}
