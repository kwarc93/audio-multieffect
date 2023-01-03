/*
 * active_object.hpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#ifndef ACTIVE_OBJECT_HPP_
#define ACTIVE_OBJECT_HPP_

#include "cmsis_os2.h"

#include <string>
#include <cassert>

namespace ao
{

template<typename T>
class active_object
{
public:
    struct event
    {
        T data;
        uint32_t flags = 0;
        enum flags { static_data = 1 << 0, dynamic_data = 1 << 1 };
        event(const T &data, uint32_t flags) : data {data}, flags {flags} {}
    };

    active_object(const std::string_view &name, osPriority_t priority, size_t stack_size)
    {
        /* Create queue of events */
        this->queue_attr.name = name.data();

        this->queue = osMessageQueueNew(32, sizeof(event*), &this->queue_attr);
        assert(this->queue != nullptr);

        /* Create worker thread */
        this->thread_attr.name = name.data();
        this->thread_attr.priority = priority;
        this->thread_attr.stack_size = stack_size;

        this->thread = osThreadNew(active_object::thread_loop, this, &this->thread_attr);
        assert(this->thread != nullptr);
    }

    virtual ~active_object()
    {
        osStatus_t status;
        status = osMessageQueueDelete(this->queue);
        assert(status == osOK);
        this->queue = nullptr;

        status = osThreadTerminate(this->thread);
        assert(status == osOK);
        this->thread = nullptr;
    }

    void send(const event &e)
    {
        assert(e.flags != 0);

        const event *evt = nullptr;

        if (e.flags & event::flags::dynamic_data)
            evt = new event(e);
        else if (e.flags & event::flags::static_data)
            evt = &e;

        assert(evt != nullptr);
        osMessageQueuePut(this->queue, &evt, 0, osWaitForever);
    }

private:
    virtual void dispatch(const event &e) = 0;

    static void thread_loop(void *arg)
    {
        active_object *this_ = static_cast<active_object*>(arg);

        while (true)
        {
            event *evt = nullptr;
            uint8_t evt_prio = 0;

            if (osMessageQueueGet(this_->queue, &evt, &evt_prio, osWaitForever) == osOK)
            {
                this_->dispatch(*evt);

                if ((*evt).flags & event::flags::dynamic_data)
                    delete evt;
            }
        }
    }

    osMessageQueueId_t queue;
    osMessageQueueAttr_t queue_attr = { 0 };
    osThreadId_t thread;
    osThreadAttr_t thread_attr = { 0 };
};

}

#endif /* ACTIVE_OBJECT_HPP_ */
