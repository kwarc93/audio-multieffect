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

#include <hal/hal_ipc.hpp>

namespace mfx
{

class ipc_effect_processor : public mfx::effect_processor_base
{
public:
    ipc_effect_processor()
    {
        const bool initialized = hal::ipc::init_cm4_to_cm7(
                                 [this]()
                                 {
                                     /* Send event to process IPC data */
                                     static const event e{ effect_processor_events::ipc_data {}, event::immutable };
                                     this->send(e, 0);
                                 });
        assert(initialized);
    }

    ~ipc_effect_processor()
    {

    }

private:
    void dispatch(const event &e) override
    {
        if (std::holds_alternative<effect_processor_events::ipc_data>(e.data))
        {
            /* IPC: receive data from CM7 */
            effect_processor_events::outgoing evt;
            const size_t bytes_received = hal::ipc::receive_from_cm7(&evt, sizeof(evt));
            if (bytes_received == sizeof(evt))
            {
                /* Notify observers about event */
                this->notify(evt);
            }
        }
        else
        {
            /* IPC: send data to CM7 */
            const size_t bytes_sent = hal::ipc::send_to_cm7((void *)&e.data, sizeof(e.data));
            if (bytes_sent != sizeof(e.data))
            {
                /* Not enough space in message buffer */
            }
        }
    }
};

}

#endif /* IPC_EFFECT_PROCESSOR_HPP_ */
