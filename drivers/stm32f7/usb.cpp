/*
 * usb.cpp
 *
 *  Created on: 15 wrz 2025
 *      Author: kwarc
 */

#include "usb.hpp"

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/gpio.hpp>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

void usb::init(usb_type type)
{
    if (type == usb_type::fs)
    {
        /* USB ID, DP, DM */
        gpio::configure({ gpio::port::porta, gpio::pin::pin10 }, gpio::mode::af, gpio::af::af10, gpio::pupd::pu, gpio::type::od);
        gpio::configure({ gpio::port::porta, gpio::pin::pin11 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::porta, gpio::pin::pin12 }, gpio::mode::af, gpio::af::af10);

        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB2, OTGFS), true);

        NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 6, 0 ));

        /* Enable VBUS sense (B device) via pin PA9 - this allows proper detection of USB connection */
        USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN;

        /* Disable B-peripheral session valid override - let hardware detect VBUS properly */
        USB_OTG_FS->GOTGCTL &= ~USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_FS->GOTGCTL &= ~USB_OTG_GOTGCTL_BVALOVAL;
    }
    else /* USB HS */
    {
        /* ULPI CLK */
        gpio::configure({ gpio::port::porta, gpio::pin::pin5 }, gpio::mode::af, gpio::af::af10);

        /* ULPI D0 D1 D2 D3 D4 D5 D6 D7 */
        gpio::configure({ gpio::port::porta, gpio::pin::pin3 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portb, gpio::pin::pin0 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portb, gpio::pin::pin1 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portb, gpio::pin::pin10 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portb, gpio::pin::pin11 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portb, gpio::pin::pin12 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portb, gpio::pin::pin13 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portb, gpio::pin::pin5 }, gpio::mode::af, gpio::af::af10);

        /* ULPI STP */
        gpio::configure({ gpio::port::portc, gpio::pin::pin0 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portc, gpio::pin::pin2 }, gpio::mode::af, gpio::af::af10);

        /* NXT */
        gpio::configure({ gpio::port::porth, gpio::pin::pin4 }, gpio::mode::af, gpio::af::af10);

        /* ULPI DIR */
        gpio::configure({ gpio::port::porti, gpio::pin::pin11 }, gpio::mode::af, gpio::af::af10);

        NVIC_SetPriority(OTG_HS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 6, 0 ));

        /* Enable USB HS & ULPI Clocks */
        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, OTGHS), true);
        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, OTGHSULPI), true);

        /* Disable VBUS sense (B device) via pin PA9 */
        USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

        /* B-peripheral session valid override enable */
        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
    }
}

//-----------------------------------------------------------------------------
/* TinyUSB DWC2 driver support functions */

extern "C"
{

// Check if USB core is high speed
bool dwc2_core_is_highspeed(void)
{
    return false; // STM32F746 only has Full Speed USB
}

// Initialize DWC2 core
void dwc2_core_init(void)
{
    // Core initialization is handled by usb::init()
}

// Read packet from RX FIFO
void dfifo_read_packet(uint8_t *buffer, uint16_t len)
{
    volatile uint32_t *fifo = (volatile uint32_t *)USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE / sizeof(uint32_t);
    
    for (uint16_t i = 0; i < (len + 3) / 4; i++)
    {
        uint32_t data = *fifo;
        buffer[i * 4] = data & 0xFF;
        if (i * 4 + 1 < len) buffer[i * 4 + 1] = (data >> 8) & 0xFF;
        if (i * 4 + 2 < len) buffer[i * 4 + 2] = (data >> 16) & 0xFF;
        if (i * 4 + 3 < len) buffer[i * 4 + 3] = (data >> 24) & 0xFF;
    }
}

// Write packet to TX FIFO
void dfifo_write_packet(uint8_t fifo_num, const uint8_t *buffer, uint16_t len)
{
    volatile uint32_t *fifo = (volatile uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + fifo_num * USB_OTG_FIFO_SIZE);
    
    for (uint16_t i = 0; i < (len + 3) / 4; i++)
    {
        uint32_t data = buffer[i * 4];
        if (i * 4 + 1 < len) data |= ((uint32_t)buffer[i * 4 + 1]) << 8;
        if (i * 4 + 2 < len) data |= ((uint32_t)buffer[i * 4 + 2]) << 16;
        if (i * 4 + 3 < len) data |= ((uint32_t)buffer[i * 4 + 3]) << 24;
        *fifo = data;
    }
}

} // extern "C"

