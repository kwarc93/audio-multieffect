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
#include <cassert>

#include <middlewares/active_object.hpp>
#include <middlewares/observer.hpp>

#include "app/model/effect_processor.hpp"

#include <hal/hal_ipc.hpp>

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
         const bool initialized = hal::ipc::init_cm7_to_cm4(
                                  [this]()
                                  {
                                      /* Send event to process IPC data */
                                      static const event e{ ipc_controller_events::ipc_data {}, event::immutable };
                                      this->send(e, 0);
                                  });

         /* Start observing model */
         this->model->attach(this);

         assert(initialized);
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
        /* IPC: send data to CM4 */
        const size_t bytes_sent = hal::ipc::send_to_cm4((void *)&e, sizeof(e));
        if (bytes_sent != sizeof(e))
        {
            /* Not enough space in message buffer */
        }
    }

    /* Event handlers */
    void event_handler(const ipc_controller_events::ipc_data &e)
    {
        /* IPC: receive data from CM4 */
        effect_processor_events::incoming evt;
        const size_t bytes_received = hal::ipc::receive_from_cm4(&evt, sizeof(evt));
        if (bytes_received == sizeof(evt))
        {
            /* Send event to model */
            this->model->send({std::move(evt)});
        }
    }

    std::unique_ptr<effect_processor_base> model;
};

}

#endif /* IPC_CONTROLLER_HPP_ */
