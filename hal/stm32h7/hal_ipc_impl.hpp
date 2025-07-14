/*
 * hal_ipc_impl.hpp
 *
 *  Created on: 21 gru 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_IPC_IMPL_HPP_
#define STM32H7_HAL_IPC_IMPL_HPP_

#ifdef DUAL_CORE_APP

#include <cstddef>

#include <drivers/stm32h7/exti.hpp>

#include "FreeRTOS.h"
#include "message_buffer.h"

namespace hal::ipc
{
    struct ipc
    {
        struct channel
        {
            volatile bool initialized;
            MessageBufferHandle_t mb_handle;
            StaticMessageBuffer_t mb_object;
            uint8_t mb_buffer[4096];
        };

        channel cm4_to_cm7;
        channel cm7_to_cm4;
    };

    inline ipc ipc_struct __attribute__((section(".ipc")));

    inline void init(void)
    {
        ipc_struct.cm4_to_cm7.initialized = false;
        ipc_struct.cm7_to_cm4.initialized = false;
    }

    inline bool init_cm4_to_cm7(std::function<void(void)> cm7_callback)
    {
        drivers::exti::configure(true,
                                 drivers::exti::line::line1,
                                 drivers::exti::port::none,
                                 drivers::exti::mode::interrupt,
                                 drivers::exti::edge::rising,
                                 cm7_callback
                                );

        ipc_struct.cm4_to_cm7.mb_handle = xMessageBufferCreateStatic(sizeof(ipc_struct.cm4_to_cm7.mb_buffer),
                                                                     ipc_struct.cm4_to_cm7.mb_buffer,
                                                                     &ipc_struct.cm4_to_cm7.mb_object);

        ipc_struct.cm4_to_cm7.initialized = ipc_struct.cm4_to_cm7.mb_handle != NULL;
        return ipc_struct.cm4_to_cm7.initialized;
    }

    inline bool init_cm7_to_cm4(std::function<void(void)> cm4_callback)
    {
        drivers::exti::configure(true,
                                 drivers::exti::line::line0,
                                 drivers::exti::port::none,
                                 drivers::exti::mode::interrupt,
                                 drivers::exti::edge::rising,
                                 cm4_callback
                                );

        ipc_struct.cm7_to_cm4.mb_handle = xMessageBufferCreateStatic(sizeof(ipc_struct.cm7_to_cm4.mb_buffer),
                                                                     ipc_struct.cm7_to_cm4.mb_buffer,
                                                                     &ipc_struct.cm7_to_cm4.mb_object);

        ipc_struct.cm7_to_cm4.initialized = ipc_struct.cm7_to_cm4.mb_handle != NULL;
        return ipc_struct.cm7_to_cm4.initialized;
    }

    inline size_t receive_from_cm7(void *data, size_t data_size, uint32_t timeout_ms = 0)
    {
        return xMessageBufferReceive(ipc_struct.cm7_to_cm4.mb_handle, data, data_size, timeout_ms);
    }

    inline size_t receive_from_cm4(void *data, size_t data_size, uint32_t timeout_ms = 0)
    {
        return xMessageBufferReceive(ipc_struct.cm4_to_cm7.mb_handle, data, data_size, timeout_ms);
    }

    inline size_t send_to_cm7(void *data, size_t data_size, uint32_t timeout_ms = 0)
    {
        while (!ipc_struct.cm7_to_cm4.initialized) { portYIELD(); };
        return xMessageBufferSend(ipc_struct.cm4_to_cm7.mb_handle, data, data_size, timeout_ms);
    }

    inline size_t send_to_cm4(void *data, size_t data_size, uint32_t timeout_ms = 0)
    {
        while (!ipc_struct.cm4_to_cm7.initialized) { portYIELD(); };
        return xMessageBufferSend(ipc_struct.cm7_to_cm4.mb_handle, data, data_size, timeout_ms);
    }

    inline void notify_cm7(void)
    {
        drivers::exti::trigger(drivers::exti::line::line0);
    }

    inline void notify_cm4(void)
    {
        drivers::exti::trigger(drivers::exti::line::line1);
    }
}

#endif /* DUAL_CORE_APP */

#endif /* STM32H7_HAL_IPC_IMPL_HPP_ */
