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
#include <vector>
#include <cassert>

namespace middlewares
{

template<typename T>
class active_object
{
public:
    struct event
    {
        T data;
        uint32_t flags;
        enum flags { immutable = 1 << 0 };
        event(const T &data, uint32_t flags = 0) : data {data}, flags {flags} {}
    };

    active_object(const std::string_view &name, osPriority_t priority, size_t stack_size, uint32_t queue_size = 32)
    {
        /* It is assumed that each active object is unique */
        assert(this->instance == nullptr);
        this->instance = this;

        /* Create queue of events */
        this->queue_attr.name = name.data();

        this->queue = osMessageQueueNew(queue_size, sizeof(event*), &this->queue_attr);
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

        for (auto timer : this->timers)
        {
            status = osTimerDelete(timer);
            assert(status == osOK);
        }

        status = osMessageQueueDelete(this->queue);
        this->queue = nullptr;

        status = osThreadTerminate(this->thread);
        assert(status == osOK);
        this->thread = nullptr;

        this->instance = nullptr;
    }

    void send(const event &e, uint32_t timeout = osWaitForever)
    {
        const event *evt = nullptr;

        if (e.flags & event::flags::immutable)
            evt = &e;
        else
            evt = new event(e);

        assert(evt != nullptr);

        osStatus_t status = osMessageQueuePut(this->queue, &evt, 0, timeout);
        assert(status == osOK);
    }

    void schedule(const event &e, uint32_t time, bool periodic)
    {
        struct timer_event : public event
        {
            active_object* target;
            osTimerId_t timer;
            timer_event(const T &data, uint32_t flags, active_object *target, osTimerId_t timer) :
            event(data, flags), target{target}, timer{timer} {}
        };

        auto *timer_evt = new timer_event(e.data, (periodic ? event::flags::immutable : 0), this, nullptr);
        assert(timer_evt != nullptr);

        auto timer_cb = [](void *arg)
                          {
                              timer_event *evt = static_cast<timer_event*>(arg);
                              evt->target->send(*evt);
                              if (!(evt->flags & event::flags::immutable))
                              {
                                  osStatus_t status = osTimerDelete(evt->timer);
                                  assert(status == osOK);
                                  delete evt;
                              }
                          };

        auto timer = osTimerNew(timer_cb, periodic ? osTimerPeriodic : osTimerOnce, timer_evt, NULL);
        assert(timer != nullptr);

        if (periodic)
            this->timers.push_back(timer);
        timer_evt->timer = timer;

        osStatus_t status = osTimerStart(timer, time);
        assert(status == osOK);
    }

    /* Used for global access (e.g. from interrupt) */
    static inline active_object *instance;
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

                if (!((*evt).flags & event::flags::immutable))
                    delete evt;
            }
        }
    }

    osMessageQueueId_t queue;
    osMessageQueueAttr_t queue_attr = { 0 };
    osThreadId_t thread;
    osThreadAttr_t thread_attr = { 0 };
    std::vector<osTimerId_t> timers;
};

}

#endif /* ACTIVE_OBJECT_HPP_ */
