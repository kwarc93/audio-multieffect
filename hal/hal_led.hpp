/*
 * hal_led.hpp
 *
 *  Created on: 25 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_LED_HPP_
#define HAL_LED_HPP_

#include <hal_interface.hpp>

#include <array>

namespace hal
{
    class led
    {
    public:
        led(hal::interface::led *interface)
        {
            this->interface = interface;
            this->brightness = 0;
        }

        virtual ~led()
        {

        }

        void set(uint8_t brightness)
        {
            this->brightness = gamma_lut[brightness];
            this->interface->set(this->brightness);
        }

        uint8_t get(void)
        {
            return this->brightness;
        }

        void set(bool state)
        {
            uint8_t brightness = state ? 255 : 0;
            this->set(brightness);
        }

    protected:
        hal::interface::led *interface;
    private:
        uint8_t brightness;

        // Gamma brightness lookup table <https://victornpb.github.io/gamma-table-generator>
        // gamma = 2.00 steps = 256 range = 0-255
        constexpr static std::array<uint8_t, 256> gamma_lut
        {
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
             1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   4,   4,
             4,   4,   5,   5,   5,   5,   6,   6,   6,   7,   7,   7,   8,   8,   8,   9,
             9,   9,  10,  10,  11,  11,  11,  12,  12,  13,  13,  14,  14,  15,  15,  16,
            16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  23,  23,  24,  24,
            25,  26,  26,  27,  28,  28,  29,  30,  30,  31,  32,  32,  33,  34,  35,  35,
            36,  37,  38,  38,  39,  40,  41,  42,  42,  43,  44,  45,  46,  47,  47,  48,
            49,  50,  51,  52,  53,  54,  55,  56,  56,  57,  58,  59,  60,  61,  62,  63,
            64,  65,  66,  67,  68,  69,  70,  71,  73,  74,  75,  76,  77,  78,  79,  80,
            81,  82,  84,  85,  86,  87,  88,  89,  91,  92,  93,  94,  95,  97,  98,  99,
           100, 102, 103, 104, 105, 107, 108, 109, 111, 112, 113, 115, 116, 117, 119, 120,
           121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137, 139, 140, 142, 143,
           145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166, 168,
           170, 171, 173, 175, 176, 178, 180, 181, 183, 185, 186, 188, 190, 192, 193, 195,
           197, 199, 200, 202, 204, 206, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224,
           226, 228, 230, 232, 233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255,
        };
    };
}

#include <hal_led_impl.hpp>

#endif /* HAL_LED_HPP_ */
