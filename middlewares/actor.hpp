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
#include <atomic>
#include <cstdio>
#include <cassert>

namespace middlewares
{

template<typename I, typename O = void*>
class actor
{
public:
    struct event
    {
        I data;
        bool immutable {false};
    };

    struct timed_event
    {
        event evt;
        std::atomic<actor*> target {nullptr};
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

    void attach(std::function<void(O)> callback)
    {
        this->callback = callback;
    }

    void notify(O evt)
    {
        if (this->callback)
            this->callback(std::move(evt));
    }

    void send(const event& evt, uint32_t timeout = portMAX_DELAY)
    {
        assert(evt.immutable);

        auto status = pdFALSE;

        if (xPortIsInsideInterrupt())
        {
            BaseType_t yield = pdFALSE;
            status = xQueueSendToBackFromISR(this->queue, &evt, &yield);
            portYIELD_FROM_ISR(yield);
        }
        else
        {
            status = xQueueSendToBack(this->queue, &evt, pdMS_TO_TICKS(timeout));
        }

        assert(status == pdTRUE);
    }

    void send(event&& evt, uint32_t timeout = portMAX_DELAY)
    {
        assert(!evt.immutable);
        assert(xPortIsInsideInterrupt() == pdFALSE);

        auto* evt_ptr = new event(std::move(evt));
        assert(evt_ptr != nullptr);

        auto status = xQueueSendToBack(this->queue, &evt_ptr, pdMS_TO_TICKS(timeout));
        assert(status == pdTRUE);
    }

    TimerHandle_t schedule(const event &evt, uint32_t period, bool periodic = true)
    {
        /* Periodic events can be immutable because they live on heap */
        auto *timer_ctx = new timed_event{{evt.data, periodic}, this};
        assert(timer_ctx != nullptr);

        auto timer_cb = [](TimerHandle_t timer)
        {
            const auto *ctx = static_cast<timed_event*>(pvTimerGetTimerID(timer));
            actor *target = ctx->target;
            const bool periodic = xTimerGetReloadMode(timer);
            const bool cancelled = target == nullptr;

            if (!cancelled)
                target->send(periodic ? ctx->evt : std::move(ctx->evt), 0);

            if (!periodic || cancelled)
            {
                auto result = xTimerDelete(timer, 0);
                assert(result == pdPASS);
                delete ctx;
            }
        };

        auto timer = xTimerCreate(nullptr, pdMS_TO_TICKS(period), periodic, timer_ctx, timer_cb);
        assert(timer != nullptr);

        auto result = xTimerStart(timer, 0);
        assert(result == pdPASS);

        return timer;
    }

    void cancel(TimerHandle_t timer)
    {
        auto *ctx = static_cast<timed_event*>(pvTimerGetTimerID(timer));
        ctx->target = nullptr;
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

    std::function<void(O)> callback {};
    QueueHandle_t queue {};
    TaskHandle_t task {};
};

}

#endif /* ACTOR_HPP_ */
