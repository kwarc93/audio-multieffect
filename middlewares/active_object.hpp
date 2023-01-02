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

template<typename T> class active_object
{
public:
    active_object(const std::string &name, osPriority_t priority, size_t stack_size)
    {
        /* Create queue of events */
        const std::string qname = name + "_queue";
        this->queue_attr.name = qname.c_str();

        this->queue = osMessageQueueNew(32, sizeof(T*), &this->queue_attr);
        assert(this->queue != nullptr);

        /* Create working thread */
        this->thread_attr.name = name.c_str();
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

    virtual void send(const T &e)
    {
        const T *event = new T(e);

        if (event != nullptr)
            osMessageQueuePut(this->queue, &event, 0, osWaitForever);
    }

private:
    virtual void dispatch(const T &e) = 0;

    static void thread_loop(void *arg)
    {
        active_object *this_ = static_cast<active_object*>(arg);

        while (true)
        {
            T *event = nullptr;
            uint8_t event_priority = 0;

            if (osMessageQueueGet(this_->queue, &event, &event_priority, osWaitForever) == osOK)
            {
                this_->dispatch(*event);
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
