/*
 * lcd_rk043fn48h.cpp
 *
 *  Created on: 20 lip 2023
 *      Author: kwarc
 */

#include "lcd_rk043fn48h.hpp"

#include <drivers/stm32f7/ltdc.hpp>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

lcd_rk043fn48h::lcd_rk043fn48h()
{
    /*
     * Configure pixel clock for LCD & LTDC
     * @note 1. Pixel clock = LCD total width x LCD total height x refresh rate
     *       2. Pixel clock source is PLLSAI output R
     */
//    const rcc::sai_pll sai_cfg
//    {
//
//    };
//
//    rcc::set_sai_pll(sai_cfg);
}
