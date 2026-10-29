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

    explicit memory_pool(std::size_t pool_size) : size(pool_size)
    {
        assert(pool_size > 0);
        
        blocks = reinterpret_cast<Block*>(pvPortMalloc(size * sizeof(Block)));
        assert(blocks != nullptr);

        head = &blocks[0];
        for (std::size_t i = 0; i + 1 < size; ++i)
        {
            blocks[i].next = &blocks[i + 1];
        }
        blocks[size - 1].next = nullptr;
        available_size = size;
    }

    ~memory_pool()
    {
        assert(available_size == size);
        if (blocks)
        {
            vPortFree(blocks);
        }
    }

    template<typename... Args>
    T* alloc(Args&&... args)
    {
        Block* block = nullptr;
        {
            detail::critical_section cs {};
            block = head;
            if (!block)
            {
                return nullptr;
            }
            head = block->next;
            --available_size;
        }

        T* obj = reinterpret_cast<T*>(block->data);
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }

    void free(T* p)
    {
        if (p == nullptr)
            return;

        p->~T();

        Block* block = reinterpret_cast<Block*>(reinterpret_cast<std::uint8_t*>(p) - offsetof(Block, data));

        detail::critical_section cs {};
        block->next = head;
        head = block;
        ++available_size;
        assert(available_size <= size);
    }

    std::size_t available() const
    {
        detail::critical_section cs {};
        return available_size;
    }

private:
    struct Block
    {
        Block* next;
        alignas(T) unsigned char data[sizeof(T)];
    };

    Block* blocks {};
    Block* head {};
    std::size_t size {};
    std::size_t available_size {};
};

}

#endif /* MEMORY_POOL_HPP_ */

