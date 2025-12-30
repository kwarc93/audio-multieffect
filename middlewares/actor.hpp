/*
 * actor.hpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#ifndef ACTOR_HPP_
#define ACTOR_HPP_

#include "cmsis_os2.h"

#include <string>
#include <cassert>

namespace middlewares
{

template<typename T>
class actor
{
public:
    struct event
    {
        event(const T &data, uint32_t flags = 0) : flags {flags}, data {data} {}
        enum flags { immutable = 1 << 0, periodic = 1 << 1 };
        uint32_t flags;
        T data;
    };

    actor(const std::string_view &name, osPriority_t priority, size_t stack_size, uint32_t queue_size = 32)
    {
        /* Create queue of events */
        this->queue_attr.name = name.data();

        this->queue = osMessageQueueNew(queue_size, sizeof(event*), &this->queue_attr);
        assert(this->queue != nullptr);

        /* Create worker thread */
        this->thread_attr.name = name.data();
        this->thread_attr.priority = priority;
        this->thread_attr.stack_size = stack_size;

        this->thread = osThreadNew(actor::thread_loop, this, &this->thread_attr);
        assert(this->thread != nullptr);
    }

    virtual ~actor()
    {
        osStatus_t status;

        status = osMessageQueueDelete(this->queue);
        assert(status == osOK);
        this->queue = nullptr;

        status = osThreadTerminate(this->thread);
        assert(status == osOK);
        this->thread = nullptr;
    }

    template <typename E = event>
    void send(E &&evt, uint32_t timeout = osWaitForever)
    {
        const event *e = nullptr;

        if (evt.flags & event::immutable)
        {
            /* rvalue must not be passed with immutable, it would cause dangling pointer */
            assert(!std::is_rvalue_reference<E&&>::value);
            e = &evt;
        }
        else
        {
            e = new event(std::forward<E>(evt));
        }

        assert(e != nullptr);

        osStatus_t status = osMessageQueuePut(this->queue, &e, 0, timeout);
        assert(status == osOK);
    }

    osTimerId_t schedule(const event &evt, uint32_t time, bool periodic)
    {
        struct timer_context
        {
            event evt;
            osTimerId_t timer;
            actor* target;
            timer_context(const T &data, uint32_t flags, actor *target, osTimerId_t timer) :
            evt{data, flags}, timer{timer}, target{target} {}
        };

        auto *timer_ctx = new timer_context(evt.data, (periodic ? event::immutable | event::periodic : 0), this, nullptr);
        assert(timer_ctx != nullptr);

        auto timer_cb = [](void *arg)
        {
            timer_context *ctx = static_cast<timer_context*>(arg);

            ctx->target->send(ctx->evt);

            if (ctx->evt.flags & event::periodic)
                return;

            /* Delete one-shot timer and its argument */
            osStatus_t status = osTimerDelete(ctx->timer);
            assert(status == osOK);
            delete ctx;
        };

        auto timer = osTimerNew(timer_cb, periodic ? osTimerPeriodic : osTimerOnce, timer_ctx, nullptr);
        assert(timer != nullptr);

        timer_ctx->timer = timer;

        osStatus_t status = osTimerStart(timer, time);
        assert(status == osOK);

        return timer;
    }

protected:
    bool wait(uint32_t flag, uint32_t timeout = osWaitForever)
    {
        return osThreadFlagsWait(flag, osFlagsWaitAny, osWaitForever) == flag;
    }

    void set(uint32_t flag, osThreadId_t thread_id = nullptr)
    {
        osThreadFlagsSet(thread_id ? thread_id : this->thread, flag);
    }

private:
    virtual void dispatch(const event &evt) = 0;

    static void thread_loop(void *arg)
    {
        actor *this_ = static_cast<actor*>(arg);

        while (true)
        {
            event *evt = nullptr;
            uint8_t evt_prio = 0;

            if (osMessageQueueGet(this_->queue, &evt, &evt_prio, osWaitForever) == osOK)
            {
                this_->dispatch(*evt);

                if (!(evt->flags & event::immutable))
                    delete evt;
            }
        }
    }


    osMessageQueueId_t queue {};
    osMessageQueueAttr_t queue_attr {};
    osThreadId_t thread {};
    osThreadAttr_t thread_attr {};
};

}

#endif /* ACTOR_HPP_ */
