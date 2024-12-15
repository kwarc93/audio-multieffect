/*
 * ipc_defs.hpp
 *
 *  Created on: 15 gru 2024
 *      Author: kwarc
 */

#ifndef IPC_DEFS_HPP_
#define IPC_DEFS_HPP_

#ifdef DUAL_CORE_APP
#include "FreeRTOS.h"
#include "message_buffer.h"

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

#endif /* DUAL_CORE_APP */

#endif /* IPC_DEFS_HPP_ */
