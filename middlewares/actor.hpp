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
        event(I value) : data(std::move(value)) {}
    private:
        actor *target {};
        friend class actor;
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

    bool send(const event &evt)
    {
        auto *e = this->mempool.alloc(evt);
        bool const result = this->enqueue(e);
        if (!result)
            this->mempool.free(e);
        return result;
    }

    bool send(event &&evt)
    {
        auto *e = this->mempool.alloc(std::move(evt));
        bool const result = this->enqueue(e);
        if (!result)
            this->mempool.free(e);
        return result;
    }

    TimerHandle_t schedule(const event &evt, uint32_t period, bool periodic = true)
    {
        event *e = this->mempool.alloc(evt);
        if (e == nullptr)
            return TimerHandle_t {};

        e->target = this;

        auto timer_cb = [](TimerHandle_t timer)
        {
            auto *e = static_cast<event*>(pvTimerGetTimerID(timer));
            const bool periodic = xTimerGetReloadMode(timer);

            e->target->send(*e);

            if (!periodic && (xTimerDelete(timer, 0) == pdPASS))
                e->target->mempool.free(e);
        };

        auto timer = xTimerCreate("actor's timer", pdMS_TO_TICKS(period), periodic, e, timer_cb);
        auto result = xTimerStart(timer, 0);

        if (timer == nullptr || result == pdFAIL)
            this->mempool.free(e);

        return timer;
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
            auto yield = pdFALSE;
            vTaskNotifyGiveFromISR(task_handle, &yield);
            portYIELD_FROM_ISR(yield);
        }
        else
        {
            xTaskNotifyGive(task_handle);
        }
    }

private:
    bool enqueue(event *evt)
    {
        if (evt == nullptr)
            return false;

        evt->target = this;

        auto status = pdFALSE;
        if (xPortIsInsideInterrupt())
        {
            auto yield = pdFALSE;
            status = xQueueSendToBackFromISR(this->queue, &evt, &yield);
            portYIELD_FROM_ISR(yield);
        }
        else
        {
            status = xQueueSendToBack(this->queue, &evt, 0);
        }

        return status == pdTRUE;
    }

    virtual void dispatch(const event &evt) = 0;

    static void thread_loop(void *arg)
    {
        auto *this_ = static_cast<actor*>(arg);

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
