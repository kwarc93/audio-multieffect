/*
 * hsem.hpp
 *
 *  Created on: 30 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HSEM_HPP_
#define STM32H7_HSEM_HPP_

#include <cstdint>

namespace drivers
{

class hsem final
{
public:
    hsem() = delete;

    static void init(void);
    static bool take(uint8_t id, uint8_t pid);
    static bool fast_take(uint8_t id);
    static bool is_taken(uint8_t id);
    static void release(uint8_t id, uint8_t pid = 0);
    static void enable_notification(uint8_t id);
    static void disable_notification(uint8_t id);

    static void irq_handler(void);
};

class hsem_spinlock
{
public:
    hsem_spinlock(uint8_t id) : hsem_id {id}
    {
        while (!hsem::fast_take(hsem_id));
    }
    ~hsem_spinlock()
    {
        hsem::release(hsem_id);
    }
private:
    const uint8_t hsem_id;
};

}
#endif /* STM32H7_HSEM_HPP_ */
