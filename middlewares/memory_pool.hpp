/*
 * memory_pool.hpp
 *
 *  Created on: 24 oct 2026
 *      Author: kwarc
 */

#ifndef MEMORY_POOL_HPP_
#define MEMORY_POOL_HPP_

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

#include "FreeRTOS.h"

namespace middlewares
{

namespace detail
{

class critical_section
{
public:
    critical_section(const critical_section&) = delete;
    critical_section& operator=(const critical_section&) = delete;
    critical_section(critical_section&&) = delete;
    critical_section& operator=(critical_section&&) = delete;

    critical_section()
    {
        interrupt_mask = xPortIsInsideInterrupt() ? taskENTER_CRITICAL_FROM_ISR() : (taskENTER_CRITICAL(), 0);
    }
    ~critical_section()
    {
        xPortIsInsideInterrupt() ? taskEXIT_CRITICAL_FROM_ISR(interrupt_mask) : taskEXIT_CRITICAL();
    }

private:
    UBaseType_t interrupt_mask {};
};

}

template<typename T>
class memory_pool
{
public:
    memory_pool(const memory_pool&) = delete;
    memory_pool& operator=(const memory_pool&) = delete;
    memory_pool(memory_pool&&) = delete;
    memory_pool& operator=(memory_pool&&) = delete;

    explicit memory_pool(std::size_t pool_size) : pool_size_(pool_size)
    {
        assert(pool_size > 0);

        blocks_ = reinterpret_cast<Block*>(pvPortMalloc(pool_size_ * sizeof(Block)));
        assert(blocks_ != nullptr);

        head_ = &blocks_[0];
        for (std::size_t i = 0; i + 1 < pool_size_; ++i)
        {
            blocks_[i].next = &blocks_[i + 1];
        }
        blocks_[pool_size_ - 1].next = nullptr;
        free_count_ = pool_size_;
    }

    ~memory_pool()
    {
        if (blocks_)
        {
            assert(free_count_ == pool_size_);
            vPortFree(blocks_);
        }
    }

    template<typename... Args>
    T* alloc(Args&&... args)
    {
        Block* blk = nullptr;
        {
            detail::critical_section cs {};
            blk = head_;
            if (!blk)
            {
                return nullptr;
            }
            head_ = blk->next;
            --free_count_;
        }

        T* obj = reinterpret_cast<T*>(blk->data);
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }

    void free(T* p)
    {
        if (p == nullptr)
            return;

        p->~T();

        Block* blk = reinterpret_cast<Block*>(reinterpret_cast<std::uint8_t*>(p) - offsetof(Block, data));

        detail::critical_section cs {};
        blk->next = head_;
        head_ = blk;
        ++free_count_;
    }

    std::size_t available() const
    {
        detail::critical_section cs {};
        return free_count_;
    }

private:
    struct Block {
        Block* next;
        alignas(T) unsigned char data[sizeof(T)];
    };

    Block* blocks_ {};
    Block* head_ {};
    std::size_t pool_size_ {};
    std::size_t free_count_ {};
};

}

#endif /* MEMORY_POOL_HPP_ */

