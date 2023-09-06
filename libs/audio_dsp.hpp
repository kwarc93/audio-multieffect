/*
 * audio_dsp.hpp
 *
 *  Created on: 6 wrz 2023
 *      Author: kwarc
 */

#ifndef AUDIO_DSP_HPP_
#define AUDIO_DSP_HPP_

#include <cstdint>
#include <algorithm>
#include <array>

/* CMSIS DSP library */
#include <cmsis/stm32f7xx.h>
#include <cmsis/dsp/arm_math.h>

namespace libs::adsp
{
    constexpr inline float pi {3.14159265359f};

    template <typename T>
    int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

//-----------------------------------------------------------------------------

class oscillator
{
public:
    enum class shape_t {sine, triangle, square, sawtooth};

    oscillator(shape_t shape, uint32_t fs) : fs{fs}
    {
        this->shape = shape;
        this->freq = 1;
        this->counter = 0;

        update();
    }
    ~oscillator() = default;

    void set_frequency(float f)
    {
        if (this->freq == f)
            return;

        this->freq = f;

        update();
    }

    void set_shape(shape_t s)
    {
        if (this->shape == s)
            return;

        this->shape = shape;

        this->counter = 0;

        update();
    }

    float generate(void)
    {
        float out = 0;

        if (this->shape == shape_t::triangle)
        {
            out = this->counter / this->counter_limit;

            this->counter_step = std::clamp(this->counter, -this->counter_limit, this->counter_limit) / -this->counter_limit;

        }
        else if (this->shape == shape_t::sine)
        {
            out = arm_sin_f32(this->counter);

            if (this->counter >= this->counter_limit)
                this->counter -= 2 * this->counter_limit;
        }

        this->counter += this->counter_step;

        return out;
    }

private:
    void update(void)
    {
        if (this->shape == shape_t::triangle)
        {
            /* One triangle slope is frequency/4 */
            this->counter_limit = 0.25f * (this->fs / this->freq);
            this->counter = std::clamp(this->counter, -this->counter_limit, this->counter_limit);

        }
        else if (this->shape == shape_t::sine)
        {
            /* Sine cycle in range [-pi : +pi] */
            this->counter_limit = pi;
            this->counter_step = 2 * this->counter_limit * this->freq / this->fs;
        }
    }

    const uint32_t fs;

    shape_t shape;
    float freq;
    float counter;
    float counter_step;
    float counter_limit;
};

//-----------------------------------------------------------------------------

class delay_line
{
public:
    delay_line(float max_delay, uint32_t fs) : fs{fs}, allocated{true}
    {
        this->memory_length = std::ceil(fs * max_delay);
        this->memory = new float [this->memory_length];
        this->write_idx = this->read_idx = 0;

        arm_fill_f32(0, this->memory, this->memory_length);
    }

    delay_line(float *samples_memory, uint32_t memory_length, uint32_t fs) : fs{fs}, allocated{false}
    {
        this->memory_length = memory_length;
        this->memory = samples_memory;
        this->write_idx = this->read_idx = 0;

        arm_fill_f32(0, this->memory, this->memory_length);
    }

    ~delay_line()
    {
        if (this->allocated)
            delete [] this->memory;
    }

    void set_delay(float d)
    {
        uint32_t delay_samples = d * this->memory_length;

        delay_samples = std::clamp(delay_samples, 0UL, this->memory_length - 1);

        if (delay_samples == 0)
            this->read_idx = this->write_idx;
        else
            this->read_idx = this->write_idx + this->memory_length - delay_samples;
    }

    float get(void)
    {
        return this->memory[this->read_idx++ % this->memory_length];
    }

    void put(float sample)
    {
        this->memory[this->write_idx++ % this->memory_length] = sample;
    }
private:
    const uint32_t fs;
    const bool allocated;

    float *memory;
    uint32_t memory_length;
    uint32_t write_idx, read_idx;

};

//-----------------------------------------------------------------------------

template<uint16_t taps, uint32_t block_size, const std::array<float, taps> &coeffs>
class fir
{
public:
    fir()
    {
        arm_fir_init_f32
        (
            &this->instance,
            coeffs.size(),
            const_cast<float*>(coeffs.data()),
            this->state.data(),
            block_size
        );
    }

    void process(const float *in, float *out)
    {
        arm_fir_f32(&this->instance, const_cast<float*>(in), out, block_size);
    }
private:
    arm_fir_instance_f32 instance;
    std::array<float, block_size + taps - 1> state;
};

class iir_biquad
{
public:
    constexpr static unsigned biquad_stages = 1;
    enum class type {lowpass, highpass};

    iir_biquad()
    {
        arm_biquad_cascade_df1_init_f32
        (
            &this->instance,
            biquad_stages,
            this->coeffs.data(),
            this->state.data()
        );
    }

    void calc_coeffs(type t, float fc, float fs)
    {
        if (t == type::lowpass)
        {
            const float wc = fc / fs;
            const float k = std::tan(pi * wc);
            const float q = 1.0f / std::sqrt(2.0f);

            const float k2q = k * k * q;
            const float denum = 1.0f / (k2q + k + q);

            /* 2-nd order */
            this->coeffs[0] = k2q * denum; // b0
            this->coeffs[1] = 2 * this->coeffs[0]; // b1
            this->coeffs[2] = this->coeffs[0]; // b2
            this->coeffs[3] = -2 * q * (k * k - 1) * denum; // -a1
            this->coeffs[4] = -(k2q - k + q) * denum; // -a2
        }
        else if (t == type::highpass)
        {
            const float wc = fc / fs;
            const float k = std::tan(pi * wc);
            const float q = 1.0f / std::sqrt(2.0f);

            const float k2q = k * k * q;
            const float denum = 1.0f / (k2q + k + q);

            /* 2-nd order */
            this->coeffs[0] = q * denum; // b0
            this->coeffs[1] = -2 * this->coeffs[0]; // b1
            this->coeffs[2] = this->coeffs[0]; // b2
            this->coeffs[3] = -2 * q * (k * k - 1) * denum; // -a1
            this->coeffs[4] = -(k2q - k + q) * denum; // -a2
        }
    }

    void process(const float *in, float *out, uint32_t samples)
    {
        arm_biquad_cascade_df1_f32(&this->instance, const_cast<float*>(in), out, samples);
    }
private:
    arm_biquad_casd_df1_inst_f32 instance;
    std::array<float, 4 * biquad_stages> state;
    std::array<float, 5 * biquad_stages> coeffs;
};

}

#endif /* AUDIO_DSP_HPP_ */
