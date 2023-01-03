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
    struct base_evt_t
    {
        T data;
        uint32_t flags = 0;
        enum flags { static_data = 1 << 0, dynamic_data = 1 << 1 };
    };

    active_object(const std::string_view &name, osPriority_t priority, size_t stack_size)
    {
        /* Create queue of events */
        this->queue_attr.name = name.data();

        this->queue = osMessageQueueNew(32, sizeof(base_evt_t*), &this->queue_attr);
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

    void send(const base_evt_t &e)
    {
        assert(e.flags != 0);

        const base_evt_t *event = nullptr;

        if (e.flags & base_evt_t::flags::dynamic_data)
            event = new base_evt_t(e);
        else if (e.flags & base_evt_t::flags::static_data)
            event = &e;

        assert(event != nullptr);
        osMessageQueuePut(this->queue, &event, 0, osWaitForever);
    }

private:
    virtual void dispatch(const T &e) = 0;

    static void thread_loop(void *arg)
    {
        active_object *this_ = static_cast<active_object*>(arg);

        while (true)
        {
            base_evt_t *event = nullptr;
            uint8_t event_priority = 0;

            if (osMessageQueueGet(this_->queue, &event, &event_priority, osWaitForever) == osOK)
            {
                this_->dispatch((*event).data);

                if ((*event).flags & base_evt_t::flags::dynamic_data)
                    delete event;
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
