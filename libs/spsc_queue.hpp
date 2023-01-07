/*
 * spsc_queue.hpp
 *
 *
 *  Created on: 7 sty 2023
 *      Author: kwarc
 */

#ifndef SPSC_QUEUE_HPP_
#define SPSC_QUEUE_HPP_

#include <array>

/* Single Producer - Single Consumer queue with no locks */
template<typename T, size_t N>
class spsc_queue
{
public:
    spsc_queue() : read_idx {0}, write_idx {0} {}
    ~spsc_queue() {}

    bool empty() const
    {
        return this->read_idx == this->write_idx;
    }

    constexpr size_t size() const
    {
        return this->write_idx - this->read_idx;
    }

    constexpr size_t max_size() const
    {
        return this->maximum_size;
    }

    bool push(const T &element)
    {
        if (this->size() == this->maximum_size)
            return false;

        this->elements[this->write_idx % this->maximum_size] = element;
        this->write_idx++;
        return true;
    }

    bool pop(T &element)
    {
        if (this->empty())
            return false;

        element = this->elements[this->read_idx % this->maximum_size];
        this->read_idx++;
        return true;
    }

private:
    /* When read_idx == write_idx queue is empty, so storage should be +1 size */
    size_t read_idx, write_idx;
    std::array<T, N + 1> elements;
    static constexpr size_t maximum_size = N - 1;
};



#endif /* SPSC_QUEUE_HPP_ */
