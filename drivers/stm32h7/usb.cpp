/*
 * usb.cpp
 *
 *  Created on: 15 wrz 2025
 *      Author: kwarc
 */

#include "usb.hpp"

#include <cmsis/stm32h7xx.h>

#include <drivers/stm32h7/gpio.hpp>
#include <drivers/stm32h7/rcc.hpp>

using namespace drivers;

void usb::init(usb_type type)
{
    if (type == usb_type::fs)
    {
        /* USB ID, DP, DM */
        gpio::configure({ gpio::port::porta, gpio::pin::pin10 }, gpio::mode::af, gpio::af::af10, gpio::pupd::pu, gpio::type::od);
        gpio::configure({ gpio::port::porta, gpio::pin::pin11 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::porta, gpio::pin::pin12 }, gpio::mode::af, gpio::af::af10);

        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, USB2OTGFS), true);

        NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 6, 0 ));

        /* Enable the USB voltage detector & VBUS sense (B device) via pin PA9*/
        gpio::configure({ gpio::port::porta, gpio::pin::pin9 }, gpio::mode::input);
        PWR->CR3 |= PWR_CR3_USB33DEN;
        USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN;
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
        //gpio::configure({ gpio::port::portc, gpio::pin::pin0 }, gpio::mode::af, gpio::af::af10);
        gpio::configure({ gpio::port::portc, gpio::pin::pin2 }, gpio::mode::af, gpio::af::af10);

        /* NXT */
        gpio::configure({ gpio::port::porth, gpio::pin::pin4 }, gpio::mode::af, gpio::af::af10);

        /* ULPI DIR */
        gpio::configure({ gpio::port::porti, gpio::pin::pin11 }, gpio::mode::af, gpio::af::af10);

        NVIC_SetPriority(OTG_HS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 6, 0 ));

        /* Enable USB HS & ULPI Clocks */
        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, USB1OTGHS), true);
        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, USB1OTGHSULPI), true);

        /* Disable VBUS sense (B device) via pin PA9 */
        USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

        /* B-peripheral session valid override enable */
        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
    }
}

