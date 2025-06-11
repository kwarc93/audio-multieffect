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
        enum flags { immutable = 1 << 0, periodic = 1 << 1 };
        event(const T &data, uint32_t flags = 0) : data {data}, flags {flags} {}
    };

    active_object(const std::string_view &name, osPriority_t priority, size_t stack_size, uint32_t queue_size = 32)
    {
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

        status = osMessageQueueDelete(this->queue);
        assert(status == osOK);
        this->queue = nullptr;

        status = osThreadTerminate(this->thread);
        assert(status == osOK);
        this->thread = nullptr;
    }

    void send(const event &e, uint32_t timeout = osWaitForever)
    {
        const event *evt = nullptr;

        evt = (e.flags & event::flags::immutable) ? &e : new event(e);
        assert(evt != nullptr);

        osStatus_t status = osMessageQueuePut(this->queue, &evt, 0, timeout);
        assert(status == osOK);
    }

    osTimerId_t schedule(const event &e, uint32_t time, bool periodic)
    {
        struct timer_context
        {
            event evt;
            osTimerId_t timer;
            active_object* target;
            timer_context(const T &data, uint32_t flags, active_object *target, osTimerId_t timer) :
            evt{data, flags}, timer{timer}, target{target} {}
        };

        auto *timer_arg = new timer_context(e.data, (periodic ? event::flags::immutable | event::flags::periodic : 0), this, nullptr);
        assert(timer_arg != nullptr);

        auto timer_cb = [](void *arg)
                          {
                              timer_context *ctx = static_cast<timer_context*>(arg);
                              ctx->target->send(ctx->evt);
                              if (!(ctx->evt.flags & event::flags::periodic))
                              {
                                  /* Delete one-shot timer and its argument */
                                  osStatus_t status = osTimerDelete(ctx->timer);
                                  assert(status == osOK);
                                  delete ctx;
                              }
                          };

        auto timer = osTimerNew(timer_cb, periodic ? osTimerPeriodic : osTimerOnce, timer_arg, NULL);
        assert(timer != nullptr);

        timer_arg->timer = timer;

        osStatus_t status = osTimerStart(timer, time);
        assert(status == osOK);

        return timer;
    }

    void cancel(osTimerId_t timer)
    {
        osStatus_t status = osTimerStop(timer);
        assert(status == osOK);

        while (osTimerIsRunning(timer))
            osThreadYield();

        delete osTimerGetArgument(timer);

        status = osTimerDelete(timer);
        assert(status == osOK);
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

                if (!((*evt).flags & event::flags::immutable))
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
