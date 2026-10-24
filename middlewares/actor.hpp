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

#include <functional>
#include <string_view>
#include <cstdio>
#include <cassert>

namespace middlewares
{

template<typename I, typename O = void*>
class actor
{
public:
    actor(const actor&) = delete;
    actor& operator=(const actor&) = delete;
    actor(actor&&) = delete;
    actor& operator=(actor&&) = delete;

    struct event
    {
        I data {};
        bool immutable {};
    };

    struct timed_event
    {
        event evt {};
        actor *target {};
    };

    actor(std::string_view name, uint32_t priority, size_t stack_size, uint32_t queue_size = 32)
    {
        assert(queue_size > 0);

        /* Create queue of events */
        this->queue = xQueueCreate(queue_size, sizeof(event*));
        assert(this->queue != nullptr);

        /* Create worker thread */
        [[maybe_unused]] auto result = pdFAIL;
        result = xTaskCreate(actor::thread_loop, name.data(), stack_size / sizeof(StackType_t), this, priority, &this->task);
        assert(result == pdPASS);
    }

    void attach(std::function<void(O)> callback)
    {
        this->callback = std::move(callback);
    }

    void notify(O evt)
    {
        if (this->callback)
            this->callback(std::move(evt));
    }

    template <typename E = event>
    void send(E &&evt, uint32_t timeout = portMAX_DELAY)
    {
        static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<E>>, event>);

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

        [[maybe_unused]] auto status = pdFALSE;

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

    TimerHandle_t schedule(const event &evt, uint32_t period, bool periodic = true)
    {
        /* Periodic events can be immutable because they live on heap */
        auto *timer_ctx = new timed_event{{evt.data, periodic}, this};
        assert(timer_ctx != nullptr);

        auto timer_cb = [](TimerHandle_t timer)
        {
            auto *ctx = static_cast<timed_event*>(pvTimerGetTimerID(timer));
            const bool periodic = xTimerGetReloadMode(timer);

            ctx->target->send(ctx->evt);

            if (!periodic)
            {
                [[maybe_unused]] auto result = xTimerDelete(timer, 0);
                assert(result == pdPASS);
                delete ctx;
            }
        };

        auto timer = xTimerCreate(nullptr, pdMS_TO_TICKS(period), periodic, timer_ctx, timer_cb);
        assert(timer != nullptr);

        [[maybe_unused]] auto result = xTimerStart(timer, 0);
        assert(result == pdPASS);

        return timer;
    }

void cancel(TimerHandle_t &timer)
    {
        assert(xPortIsInsideInterrupt() == pdFALSE);
        if (timer == nullptr)
            return;

        auto *ctx = static_cast<timed_event*>(pvTimerGetTimerID(timer));
        if (ctx != nullptr)
        {
            [[maybe_unused]] auto result = xTimerDelete(timer, portMAX_DELAY);
            assert(result == pdPASS);

            result = xTimerPendFunctionCall(
            [](void *p, uint32_t)
            {
                delete static_cast<timed_event*>(p);
            },
            ctx, 0, portMAX_DELAY);
            assert(result == pdPASS);
        }

        timer = nullptr;
    }
protected:
    virtual ~actor()
    {
        assert(!"actor must not be destroyed");
    }

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

    std::function<void(O)> callback {};
    QueueHandle_t queue {};
    TaskHandle_t task {};
};

}

#endif /* ACTOR_HPP_ */
