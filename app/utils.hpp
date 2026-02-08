/*
 * utils.hpp
 *
 *  Created on: 28 sie 2023
 *      Author: kwarc
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <algorithm>
#include <type_traits>

namespace mfx::utils
{

template <typename I, typename O, typename V,
typename = std::enable_if_t<std::is_arithmetic<I>::value && std::is_arithmetic<O>::value && std::is_arithmetic<V>::value>>
constexpr std::common_type_t<I, O, V>
remap(I in_start, I in_end, O out_start, O out_end, V in_value) noexcept
{
    using common_t = std::common_type_t<I, O, V>;
    using calc_t = std::conditional_t<std::is_floating_point<common_t>::value, common_t, float>;

    const calc_t in_s  = static_cast<calc_t>(in_start);
    const calc_t in_e  = static_cast<calc_t>(in_end);
    const calc_t out_s = static_cast<calc_t>(out_start);
    const calc_t out_e = static_cast<calc_t>(out_end);
    const calc_t val   = static_cast<calc_t>(in_value);

    if (in_e == in_s) return static_cast<common_t>(out_s);

    calc_t result = (val - in_s) * (out_e - out_s) / (in_e - in_s) + out_s;
    return static_cast<common_t>(result);
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
