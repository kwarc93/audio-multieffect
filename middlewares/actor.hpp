/*
 * actor.hpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#ifndef ACTOR_HPP_
#define ACTOR_HPP_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

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
        T data;
        bool immutable {false};
    };

    actor(const std::string_view &name, uint32_t priority, size_t stack_size, uint32_t queue_size = 32)
    {
        /* Create queue of events */
        this->queue = xQueueCreate(queue_size, sizeof(event*));
        assert(this->queue != nullptr);

        /* Create worker thread */
        auto result = xTaskCreate(actor::thread_loop, name.data(), stack_size / sizeof(StackType_t), this, priority, &this->task);
        assert(result == pdPASS);
    }

    virtual ~actor()
    {
        vQueueDelete(this->queue);
        this->queue = nullptr;

        vTaskDelete(this->task);
        this->task = nullptr;
    }

    template <typename E = event>
    void send(E &&evt, uint32_t timeout = portMAX_DELAY)
    {
        const event *e = nullptr;

        if (evt.immutable)
        {
            /* With immutable (static) events, rvalue must not be passed - it would cause dangling pointer */
            assert(!std::is_rvalue_reference<E&&>::value);
            e = &evt;
        }
        else
        {
            /* Mutable (dynamic) events must not be used from interrupt */
            assert(xPortIsInsideInterrupt() == pdFALSE);
            e = new event(std::forward<E>(evt));
        }

        assert(e != nullptr);

        auto status = pdFALSE;

        if (xPortIsInsideInterrupt())
        {
            BaseType_t yield = pdFALSE;
            status = xQueueSendToBackFromISR(this->queue, &e, &yield);
            portYIELD_FROM_ISR(yield);
        }
        else
        {
            status = xQueueSendToBack(this->queue, &e, pdMS_TO_TICKS(timeout));
        }

        assert(status == pdTRUE);
    }

    void schedule(const event &evt, uint32_t time, bool periodic)
    {
        struct timer_context
        {
            event evt;
            actor* target;
        };

        auto *timer_ctx = new timer_context{{evt.data, periodic}, this};
        assert(timer_ctx != nullptr);

        auto timer_cb = [](TimerHandle_t timer)
        {
            auto *ctx = static_cast<timer_context*>(pvTimerGetTimerID(timer));

            ctx->target->send(ctx->evt);

            if (ctx->evt.immutable)
                return;

            /* Delete one-shot timer and its argument */
            auto result = xTimerDelete(timer, 0);
            assert(result == pdPASS);
            delete ctx;
        };

        auto timer = xTimerCreate(nullptr, pdMS_TO_TICKS(time), periodic, timer_ctx, timer_cb);
        assert(timer != nullptr);

        auto result = xTimerStart(timer, 0);
        assert(result == pdPASS);
    }

protected:
    bool wait(uint32_t timeout = portMAX_DELAY)
    {
        if (xPortIsInsideInterrupt())
            return false;

        return ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout)) > 0;
    }

    void signal(TaskHandle_t task_handle = nullptr)
    {
        task_handle = task_handle ? task_handle : this->task;

        if (xPortIsInsideInterrupt())
        {
            BaseType_t yield = pdFALSE;
            vTaskNotifyGiveFromISR(task_handle, &yield);
            portYIELD_FROM_ISR(yield);
        }
        else
        {
            xTaskNotifyGive(task_handle);
        }
    }

private:
    virtual void dispatch(const event &evt) = 0;

    static void thread_loop(void *arg)
    {
        actor *this_ = static_cast<actor*>(arg);

        printf("Actor '%s' started, event size: %u bytes\r\n", pcTaskGetName(this_->task), sizeof(event));

        while (true)
        {
            event *evt = nullptr;

            if (xQueueReceive(this_->queue, &evt, portMAX_DELAY) == pdTRUE)
            {
                this_->dispatch(*evt);

                if (!evt->immutable)
                    delete evt;
            }
        }
    }

    QueueHandle_t queue {};
    TaskHandle_t task {};
};

}

#endif /* ACTOR_HPP_ */
