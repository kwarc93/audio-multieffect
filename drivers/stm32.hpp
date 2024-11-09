/*
 * stm32.hpp
 *
 *  Created on: 9 lis 2024
 *      Author: kwarc
 */

#ifndef STM32_HPP_
#define STM32_HPP_

#if defined(STM32F7)
#include <cmsis/stm32f7xx.h>
#include <drivers/stm32f7/core.hpp>
#include <drivers/stm32f7/delay.hpp>
#include <drivers/stm32f7/dma2d.hpp>
#include <drivers/stm32f7/exti.hpp>
#include <drivers/stm32f7/flash.hpp>
#include <drivers/stm32f7/fmc.hpp>
#include <drivers/stm32f7/gpio.hpp>
#include <drivers/stm32f7/i2c_sw.hpp>
#include <drivers/stm32f7/i2c.hpp>
#include <drivers/stm32f7/ltdc.hpp>
#include <drivers/stm32f7/one_wire.hpp>
#include <drivers/stm32f7/qspi.hpp>
#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/rng.hpp>
#include <drivers/stm32f7/sai.hpp>
#include <drivers/stm32f7/timer.hpp>
#include <drivers/stm32f7/usart.hpp>
#elif defined(STM32H7)
#include <cmsis/stm32h7xx.h>
#include <drivers/stm32h7/core.hpp>
#include <drivers/stm32h7/delay.hpp>
//#include <drivers/stm32h7/dma2d.hpp>
//#include <drivers/stm32h7/exti.hpp>
#include <drivers/stm32h7/flash.hpp>
#include <drivers/stm32h7/fmc.hpp>
#include <drivers/stm32h7/gpio.hpp>
//#include <drivers/stm32h7/i2c_sw.hpp>
//#include <drivers/stm32h7/i2c.hpp>
//#include <drivers/stm32h7/ltdc.hpp>
//#include <drivers/stm32h7/one_wire.hpp>
//#include <drivers/stm32h7/qspi.hpp>
#include <drivers/stm32h7/rcc.hpp>
//#include <drivers/stm32h7/rng.hpp>
//#include <drivers/stm32h7/sai.hpp>
//#include <drivers/stm32h7/timer.hpp>
#include <drivers/stm32h7/usart.hpp>
#else
#error Missing STM32 family
#endif



#endif /* STM32_HPP_ */
