/*
 * ipc_controller.hpp
 *
 *  Created on: 15 gru 2024
 *      Author: kwarc
 */

#ifndef IPC_CONTROLLER_HPP_
#define IPC_CONTROLLER_HPP_

#include <variant>
#include <memory>

#include <middlewares/active_object.hpp>
#include <middlewares/observer.hpp>

#include "app/model/effect_processor.hpp"
#include "app/ipc_defs.hpp"

#include <drivers/stm32h7/exti.hpp>

namespace mfx
{

namespace ipc_controller_events
{

struct ipc_data
{

};

using incoming = std::variant
<
    ipc_data
>;

}

class ipc_controller : public middlewares::active_object<ipc_controller_events::incoming>,
                       public middlewares::observer<effect_processor_events::outgoing>
{
public:
     ipc_controller(std::unique_ptr<effect_processor_base> model) :
     active_object("ipc_ctrl", osPriorityNormal, 2048),
     model {std::move(model)}
     {
         ipc_struct.cm7_to_cm4.mb_handle = xMessageBufferCreateStatic(sizeof(ipc_struct.cm7_to_cm4.mb_buffer),
                                                                      ipc_struct.cm7_to_cm4.mb_buffer,
                                                                      &ipc_struct.cm7_to_cm4.mb_object);
         assert(ipc_struct.cm7_to_cm4.mb_handle != NULL);

         drivers::exti::configure(true,
                                  drivers::exti::line::line0,
                                  drivers::exti::port::none,
                                  drivers::exti::mode::interrupt,
                                  drivers::exti::edge::rising,
                                  [this]()
                                  {
                                     /* Send event to process IPC data */
                                     static const event e{ ipc_controller_events::ipc_data {}, event::immutable };
                                     this->send(e, 0);
                                  }
                                 );

         /* Start observing model */
         this->model->attach(this);
     }

    ~ipc_controller()
    {

    }

private:
    void dispatch(const event &e) override
    {
        std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
    }

    void update(const effect_processor_events::outgoing &e) override
    {
        const size_t bytes_sent = xMessageBufferSend(ipc_struct.cm7_to_cm4.mb_handle, (void *)&e, sizeof(e), 0);
        if (bytes_sent != sizeof(e))
        {
            /* Not enough space in message buffer */
        }
    }

    /* Event handlers */
    void event_handler(const ipc_controller_events::ipc_data &e)
    {
        effect_processor_events::incoming evt;
        const size_t bytes_received = xMessageBufferReceive(ipc_struct.cm4_to_cm7.mb_handle, &evt, sizeof(evt), 0);

        /* Check the number of bytes received was as expected. */
        if (bytes_received == sizeof(evt))
        {
            /* Send event to model */
            this->model->send(evt);
        }
    }

    std::unique_ptr<effect_processor_base> model;
};

}

void generate_cm4_interrupt( void * xUpdatedMessageBuffer )
{
    MessageBufferHandle_t updated_buffer = (MessageBufferHandle_t) xUpdatedMessageBuffer;

    if (updated_buffer == ipc_struct.cm7_to_cm4.mb_handle)
    {
        drivers::exti::trigger(drivers::exti::line::line1);
    }
}

#endif /* IPC_CONTROLLER_HPP_ */
