/*
 * utils.hpp
 *
 *  Created on: 28 sie 2023
 *      Author: kwarc
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <algorithm>

namespace mfx::utils
{

template<typename T>
constexpr T map_range(T in_start, T in_end, T out_start, T out_end, T in_value)
{
    const T in_range = in_end - in_start;
    const T out_range = out_end - out_start;

    return (in_value - in_start) * out_range / in_range + out_start;
}

constexpr float lin_to_log(float x)
{
    /* Approx. log curve by two lines */
    x = std::clamp(x, 0.0f, 1.0f);
    return (x < 0.5f) ? x * 0.2f : -0.8f + x * 1.8f;
}

constexpr float lin_to_inv_log(float x)
{
    /* Approx. inv. log curve by two lines */
    x = std::clamp(x, 0.0f, 1.0f);
    return (x < 0.5f) ? x * 1.8f : 0.8f + x * 0.2f;
}

constexpr float log_to_lin(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return (x < 0.1f) ? x * 5.0f : (1.0f/2.25f) + x * (1.0f/1.8f);
}

constexpr float inv_log_to_lin(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return (x < 0.9f) ? x * (1.0f/1.8f) : -4.0f + x * 5.0f;
}

}

#endif /* UTILS_HPP_ */
