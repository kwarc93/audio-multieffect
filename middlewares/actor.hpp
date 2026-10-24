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

#include "memory_pool.hpp"

#include <functional>
#include <string_view>
#include <cstddef>
#include <new>
#include <cstdio>
#include <cassert>
#include <utility>

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

    using event = I;

    struct timed_event
    {
        event evt {};
        actor *target {};
    };

    actor(std::string_view name, uint32_t priority, size_t stack_size, uint32_t queue_size = 32) : mempool(queue_size)
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

    void send(const event &evt)
    {
        this->enqueue(this->mempool.alloc(evt));
    }

    void send(event &&evt)
    {
        this->enqueue(this->mempool.alloc(std::move(evt)));
    }

    TimerHandle_t schedule(const event &evt, uint32_t period, bool periodic = true)
    {
        auto *timer_ctx = new timed_event{evt, this};
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
                [](void *p1, uint32_t)
                {
                    delete static_cast<timed_event*>(p1);
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
    void enqueue(event *evt)
    {
        assert(evt != nullptr);

        BaseType_t status = pdFALSE;
        if (xPortIsInsideInterrupt())
        {
            BaseType_t yield = pdFALSE;
            status = xQueueSendToBackFromISR(this->queue, &evt, &yield);
            portYIELD_FROM_ISR(yield);
        }
        else
        {
            status = xQueueSendToBack(this->queue, &evt, 0);
        }

        if (status != pdTRUE)
            this->mempool.free(evt);
    }

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
                this_->mempool.free(evt);
            }
        }
    }

    std::function<void(O)> callback {};
    memory_pool<event> mempool;
    QueueHandle_t queue {};
    TaskHandle_t task {};
};

}

#endif /* ACTOR_HPP_ */
