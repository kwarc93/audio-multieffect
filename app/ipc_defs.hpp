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
    MessageBufferHandle_t cm4_to_cm7_handle;
    StaticMessageBuffer_t cm4_to_cm7_struct;
    uint8_t cm4_to_cm7_buf[4096];
};

inline ipc ipc_struct __attribute__((section(".ipc")));

#endif /* DUAL_CORE_APP */

#endif /* IPC_DEFS_HPP_ */
