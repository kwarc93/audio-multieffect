/*
 * ipc_effect_processor.hpp
 *
 *  Created on: 15 gru 2024
 *      Author: kwarc
 */

#ifndef IPC_EFFECT_PROCESSOR_HPP_
#define IPC_EFFECT_PROCESSOR_HPP_

#include <cassert>

#include "app/model/effect_processor.hpp"
#include "app/ipc_defs.hpp"

#include <drivers/stm32h7/exti.hpp>

namespace mfx
{

class ipc_effect_processor : public mfx::effect_processor_base
{
public:
    ipc_effect_processor()
    {
        ipc_struct.cm4_to_cm7_handle = xMessageBufferCreateStatic(sizeof(ipc_struct.cm4_to_cm7_buf), ipc_struct.cm4_to_cm7_buf, &ipc_struct.cm4_to_cm7_struct);
        assert(ipc_struct.cm4_to_cm7_handle != NULL);
    }

    ~ipc_effect_processor()
    {

    }

private:
    void dispatch(const event &e) override
    {
        const size_t bytes_sent = xMessageBufferSend(ipc_struct.cm4_to_cm7_handle, (void *)&e.data, sizeof(e.data), 0);
        if (bytes_sent != sizeof(e.data))
        {
            /* Not enough space in message buffer */
        }
    }
};

}

void generate_cm7_interrupt(void * xUpdatedMessageBuffer)
{
    MessageBufferHandle_t updated_buffer = (MessageBufferHandle_t) xUpdatedMessageBuffer;

    if (updated_buffer == ipc_struct.cm4_to_cm7_handle)
    {
        drivers::exti::trigger(drivers::exti::line::line0);
    }
}

#endif /* IPC_EFFECT_PROCESSOR_HPP_ */
