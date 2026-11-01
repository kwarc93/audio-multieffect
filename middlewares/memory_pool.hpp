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

#include <hal_system.hpp>

namespace middlewares
{

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
        
        blocks = new (std::nothrow) Block[size];
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
            delete[] blocks;
        }
    }

    template<typename... Args>
    T* alloc(Args&&... args)
    {
        Block* block = nullptr;
        {
            hal::system::critical_section cs {};
            block = head;
            if (!block)
                return nullptr;
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

        [[maybe_unused]] const auto data_begin = reinterpret_cast<std::uintptr_t>(blocks) + offsetof(Block, data);
        [[maybe_unused]] const auto address = reinterpret_cast<std::uintptr_t>(p);
        assert(address >= data_begin && address - data_begin < size * sizeof(Block) && (address - data_begin) % sizeof(Block) == 0);

        p->~T();

        Block* block = reinterpret_cast<Block*>(reinterpret_cast<std::uint8_t*>(p) - offsetof(Block, data));
        {
            hal::system::critical_section cs {};
            block->next = head;
            head = block;
            ++available_size;
            assert(available_size <= size);
        }
    }

    std::size_t available() const
    {
        hal::system::critical_section cs {};
        return available_size;
    }

private:
    struct Block
    {
        Block* next;
        alignas(T) unsigned char data[sizeof(T)];
    };

    Block* head {};
    Block* blocks {};
    const std::size_t size {};
    std::size_t available_size {};
};

}

#endif /* MEMORY_POOL_HPP_ */
