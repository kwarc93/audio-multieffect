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

#include "FreeRTOS.h"
#include "message_buffer.h"

#include <drivers/stm32h7/exti.hpp>

namespace hal::ipc
{
    struct ipc
    {
        struct channel
        {
            MessageBufferHandle_t mb_handle;
            StaticMessageBuffer_t mb_object;
            uint8_t mb_buffer[4096];
        };

        channel cm4_to_cm7;
        channel cm7_to_cm4;
    };

    inline ipc ipc_struct __attribute__((section(".ipc")));

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

        return ipc_struct.cm4_to_cm7.mb_handle != NULL;
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

        return ipc_struct.cm7_to_cm4.mb_handle != NULL;
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
        return xMessageBufferSend(ipc_struct.cm4_to_cm7.mb_handle, data, data_size, timeout_ms);
    }

    inline size_t send_to_cm4(void *data, size_t data_size, uint32_t timeout_ms = 0)
    {
        return xMessageBufferSend(ipc_struct.cm7_to_cm4.mb_handle, data, data_size, timeout_ms);
    }
}

void ipc_notify_cm7(void * xUpdatedMessageBuffer)
{
    MessageBufferHandle_t updated_buffer = static_cast<MessageBufferHandle_t>(xUpdatedMessageBuffer);

    if (updated_buffer == hal::ipc::ipc_struct.cm4_to_cm7.mb_handle)
    {
        drivers::exti::trigger(drivers::exti::line::line0);
    }
}

void ipc_notify_cm4( void * xUpdatedMessageBuffer )
{
    MessageBufferHandle_t updated_buffer = static_cast<MessageBufferHandle_t>(xUpdatedMessageBuffer);

    if (updated_buffer == hal::ipc::ipc_struct.cm7_to_cm4.mb_handle)
    {
        drivers::exti::trigger(drivers::exti::line::line1);
    }
}

#endif /* DUAL_CORE_APP */

#endif /* STM32H7_HAL_IPC_IMPL_HPP_ */
