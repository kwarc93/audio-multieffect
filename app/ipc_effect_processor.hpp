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
        ipc_struct.cm4_to_cm7.mb_handle = xMessageBufferCreateStatic(sizeof(ipc_struct.cm4_to_cm7.mb_buffer),
                                                                     ipc_struct.cm4_to_cm7.mb_buffer,
                                                                     &ipc_struct.cm4_to_cm7.mb_object);
        assert(ipc_struct.cm4_to_cm7.mb_handle != NULL);

        drivers::exti::configure(true,
                                 drivers::exti::line::line1,
                                 drivers::exti::port::none,
                                 drivers::exti::mode::interrupt,
                                 drivers::exti::edge::rising,
                                 [this]()
                                 {
                                    /* Send event to process IPC data */
                                    static const event e{ effect_processor_events::process_data {}, event::immutable };
                                    this->send(e, 0);
                                 }
                                );
    }

    ~ipc_effect_processor()
    {

    }

private:
    void dispatch(const event &e) override
    {
        if (std::holds_alternative<effect_processor_events::process_data>(e.data))
        {
            this->event_handler(std::get<effect_processor_events::process_data>(e.data));
        }
        else
        {
            const size_t bytes_sent = xMessageBufferSend(ipc_struct.cm4_to_cm7.mb_handle, (void *)&e.data, sizeof(e.data), 0);
            if (bytes_sent != sizeof(e.data))
            {
                /* Not enough space in message buffer */
            }
        }
    }

    /* Event handlers */
    void event_handler(const effect_processor_events::process_data &e)
    {
        effect_processor_events::outgoing evt;
        const size_t bytes_received = xMessageBufferReceive(ipc_struct.cm7_to_cm4.mb_handle, &evt, sizeof(evt), 0);

        /* Check the number of bytes received was as expected. */
        if (bytes_received == sizeof(evt))
        {
            /* Notify observers about event */
            this->notify(evt);
        }
    }
};

}

void generate_cm7_interrupt(void * xUpdatedMessageBuffer)
{
    MessageBufferHandle_t updated_buffer = (MessageBufferHandle_t) xUpdatedMessageBuffer;

    if (updated_buffer == ipc_struct.cm4_to_cm7.mb_handle)
    {
        drivers::exti::trigger(drivers::exti::line::line0);
    }
}

#endif /* IPC_EFFECT_PROCESSOR_HPP_ */
