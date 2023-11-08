/*
 * audio_dsp.hpp
 *
 *  Created on: 6 wrz 2023
 *      Author: kwarc
 */

#ifndef AUDIO_DSP_HPP_
#define AUDIO_DSP_HPP_

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>
#include <random>

/* CMSIS DSP library */
#include <cmsis/stm32f7xx.h>
#include <cmsis/dsp/arm_math.h>

//-----------------------------------------------------------------------------

namespace libs::adsp
{
    constexpr inline float pi {3.1415926535897932};

    template <typename T>
    int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

//-----------------------------------------------------------------------------

class random
{
public:
    random(float min, float max) : gen{std::random_device{}()}, dis{min, max} {}

    float operator()()
    {
        return this->dis(this->gen);
    }

    float operator()(float min, float max)
    {
        return decltype(this->dis){min, max}(this->gen);
    }

private:
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
};

//-----------------------------------------------------------------------------

class oscillator
{
public:
    enum class shape {sawtooth, square, triangle, sine, noise};

    oscillator(shape shape, uint32_t fs) : fs{fs}
    {
        this->wave_shape = shape;
        this->frequency = 1;

        this->counter = 0;
        this->counter_limit = this->fs / this->frequency;
    }

    void set_frequency(float f)
    {
        if (this->frequency == f)
            return;

        this->frequency = f;
        this->counter_limit = this->fs / this->frequency;
    }

    void set_shape(shape s)
    {
        if (this->wave_shape == s)
            return;

        this->wave_shape = wave_shape;
    }

    float generate(void)
    {
        /* Unipolar sawtooth counter */
        this->counter %= this->counter_limit;
        float out = static_cast<float>(this->counter++ % this->counter_limit) / this->counter_limit;

        if (this->wave_shape == shape::sawtooth)
        {
            out = 2 * (out - 0.5f);
        }
        else if (this->wave_shape == shape::square)
        {
            out = (2 * (out - 0.5f) >= 0) ? 1 : -1;
        }
        else if (this->wave_shape == shape::triangle)
        {
            out = -4 * std::abs(out - 0.5f) + 1;
        }
        else if (this->wave_shape == shape::sine)
        {
            out = arm_sin_f32(-2 * pi * out + pi);
        }
        else if (this->wave_shape == shape::noise)
        {
            /* TODO */
        }

        return out;
    }

private:
    const uint32_t fs;

    shape wave_shape;
    float frequency;
    uint32_t counter;
    uint32_t counter_limit;
};

//-----------------------------------------------------------------------------

enum class delay_line_intrpl {none, linear, allpass};

template<delay_line_intrpl intrpl = delay_line_intrpl::none>
class delay_line
{
public:

    delay_line(float max_delay, uint32_t fs) : fs{fs}, allocated{true}
    {
        this->memory_length = std::ceil(fs * max_delay) + 1;
        this->memory = new float [this->memory_length];
        this->write_idx = this->read_idx = 0;
        this->frac = 0;
        this->aph = 0;

        arm_fill_f32(0, this->memory, this->memory_length);
    }

    delay_line(float *samples_memory, uint32_t memory_length, uint32_t fs) : fs{fs}, allocated{false}
    {
        this->memory_length = memory_length;
        this->memory = samples_memory;
        this->write_idx = this->read_idx = 0;
        this->frac = 0;
        this->aph = 0;

        arm_fill_f32(0, this->memory, this->memory_length);
    }

    ~delay_line()
    {
        if (this->allocated)
            delete [] this->memory;
    }

    void set_delay(float d)
    {
        const float delay_samples = d * this->fs;
        const uint32_t delay_samples_int = delay_samples;

        this->frac = delay_samples - delay_samples_int;
        this->read_idx = this->write_idx - delay_samples_int;
    }

    float get(void)
    {
        if constexpr (intrpl == delay_line_intrpl::linear)
        {
            const float s1 = this->memory[(this->read_idx - 1) % this->memory_length];
            const float s0 = this->memory[this->read_idx++ % this->memory_length];
            return s0 + this->frac * (s1 - s0);
        }
        else if constexpr (intrpl == delay_line_intrpl::allpass)
        {
            const float s1 = this->memory[(this->read_idx - 1) % this->memory_length];
            const float s0 = this->memory[this->read_idx++ % this->memory_length];
            const float d = (1 - this->frac) / (1 + this->frac); // Or just '(1 - this->frac)'
            return this->aph = s1 + d * (s0 - this->aph);
        }
        else
        {
            return this->memory[this->read_idx++ % this->memory_length];
        }
    }

    float at(float d)
    {
        const uint32_t read_idx = this->write_idx - static_cast<uint32_t>(d * this->fs);
        return this->memory[read_idx % this->memory_length];
    }

    void put(float sample)
    {
        this->memory[++this->write_idx % this->memory_length] = sample;
    }
private:
    const uint32_t fs;
    const bool allocated;

    float *memory;
    uint32_t memory_length;
    uint32_t write_idx, read_idx;
    float frac;
    float aph;
};

//-----------------------------------------------------------------------------

template<uint16_t taps, const std::array<float, taps> &coeffs, uint32_t block_size>
class fir
{
public:
    fir()
    {
        arm_fir_init_f32
        (
            &this->instance,
            taps,
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

//-----------------------------------------------------------------------------

template<uint8_t biquad_stages>
class iir_biquad
{
public:
    iir_biquad()
    {
        arm_biquad_cascade_df2T_init_f32
        (
            &this->instance,
            biquad_stages,
            this->coeffs.data(),
            this->state.data()
        );
    }

    void set_coeffs(const std::array<float, 5 * biquad_stages> &coeffs)
    {
        this->coeffs = coeffs;
    }

    void process(const float *in, float *out, uint32_t samples)
    {
        arm_biquad_cascade_df2T_f32(&this->instance, const_cast<float*>(in), out, samples);
    }
protected:
    arm_biquad_cascade_df2T_instance_f32 instance;
    std::array<float, 2 * biquad_stages> state;
    std::array<float, 5 * biquad_stages> coeffs;
};

class iir_lowpass : public iir_biquad<1>
{
public:
    void calc_coeffs(float fc, float fs, bool first_order = false)
    {
        const float wc = fc / fs;
        const float k = std::tan(pi * wc);
        const float q = 1.0f / std::sqrt(2.0f);

        const float k2q = k * k * q;

        if (first_order)
        {
            /* 1-st order */
            const float denum = 1.0f / (k + 1);

            this->coeffs[0] = k * denum; // b0
            this->coeffs[1] = this->coeffs[0]; // b1
            this->coeffs[2] = 0; //b2
            this->coeffs[3] = -(k - 1) * denum; // -a1
            this->coeffs[4] = 0; // -a2
        }
        else
        {
            /* 2-nd order */
            const float denum = 1.0f / (k2q + k + q);

            this->coeffs[0] = k2q * denum; // b0
            this->coeffs[1] = 2 * this->coeffs[0]; // b1
            this->coeffs[2] = this->coeffs[0]; // b2
            this->coeffs[3] = -2 * q * (k * k - 1) * denum; // -a1
            this->coeffs[4] = -(k2q - k + q) * denum; // -a2
        }
    }
};

class iir_highpass : public iir_biquad<1>
{
public:
    void calc_coeffs(float fc, float fs, bool first_order = false)
    {
        const float wc = fc / fs;
        const float k = std::tan(pi * wc);
        const float q = 1.0f / std::sqrt(2.0f);

        const float k2q = k * k * q;

        if (first_order)
        {
            /* 1-st order */
            const float denum = 1.0f / (k + 1);

            this->coeffs[0] = denum; // b0
            this->coeffs[1] = -this->coeffs[0]; // b1
            this->coeffs[2] = 0; //b2
            this->coeffs[3] = -(k - 1) * denum; // -a1
            this->coeffs[4] = 0; // -a2
        }
        else
        {
            /* 2-nd order */
            const float denum = 1.0f / (k2q + k + q);

            this->coeffs[0] = q * denum; // b0
            this->coeffs[1] = -2 * this->coeffs[0]; // b1
            this->coeffs[2] = this->coeffs[0]; // b2
            this->coeffs[3] = -2 * q * (k * k - 1) * denum; // -a1
            this->coeffs[4] = -(k2q - k + q) * denum; // -a2
        }
    }
};

class iir_allpass : public iir_biquad<1>
{
public:
    void calc_coeffs(float fc, float fb, float fs, bool first_order = false)
    {
        const float wc = fc / fs;
        const float k = std::tan(pi * wc);
        const float q = fc / fb;

        const float k2q = k * k * q;

        if (first_order)
        {
            /* 1-st order */
            const float denum = 1.0f / (k + 1);

            this->coeffs[0] = (k - 1) * denum; // b0
            this->coeffs[1] = 1; // b1
            this->coeffs[2] = 0; //b2
            this->coeffs[3] = -this->coeffs[0]; // -a1
            this->coeffs[4] = 0; // -a2
        }
        else
        {
            /* 2-nd order */
            const float denum = 1.0f / (k2q + k + q);

            this->coeffs[0] = (k2q - k + q) * denum; // b0
            this->coeffs[1] = 2 * q * (k * k - 1) * denum; // b1
            this->coeffs[2] = 1; // b2
            this->coeffs[3] = -this->coeffs[1]; // -a1
            this->coeffs[4] = -this->coeffs[0]; // -a2
        }
    }
};

//-----------------------------------------------------------------------------

template<uint16_t block_size, uint16_t ir_size>
class fast_convolution
{
public:
    fast_convolution()
    {
        arm_rfft_fast_init_f32(&this->fft, this->fft_size);
        arm_fill_f32(0, this->ir_fft.data(), this->ir_fft.size());
        arm_fill_f32(0, this->input.data(), this->input.size());
    }

    void set_ir(const float *ir)
    {
        /* Precompute FFT of IR */
        arm_fill_f32(0, this->ir_fft.data(), this->ir_fft.size());
        arm_copy_f32(const_cast<float*>(ir), this->ir_fft.data(), ir_size);
        arm_rfft_fast_f32(&this->fft, this->ir_fft.data(), this->ir_fft.data() + this->fft_size, 0);
    }

    void process(const float *in, float *out)
    {
        /* Overlap-save fast convolution */

        const uint32_t move_size = this->input.size() - block_size;

        /* Sliding window of input blocks */
        arm_copy_f32(this->input.data() + block_size, this->input.data(), move_size);
        arm_copy_f32(const_cast<float*>(in), this->input.data() + move_size, block_size);

        /* FFT of sliding window */
        arm_copy_f32(this->input.data(), this->input_fft.data(), this->input.size());
        arm_rfft_fast_f32(&this->fft, this->input_fft.data(), this->input_fft.data() + this->fft_size, 0);

        /* Multiplication (circular convolution) in frequency domain */
        arm_cmplx_mult_cmplx_f32(this->ir_fft.data() + this->fft_size, this->input_fft.data() + this->fft_size, this->input_fft.data(), this->fft_size / 2);

        /* Inverse FFT */
        arm_rfft_fast_f32(&this->fft, this->input_fft.data(), this->input_fft.data() + this->fft_size, 1);

        /* Copy result to output */
        arm_copy_f32(this->input_fft.data() + this->input_fft.size() - block_size, out, block_size);
    }
private:
    /* Max supported FFT size is 4096 */
    static_assert((block_size + ir_size) <= 4096);

    /* Ceil FFT size to next power of 2 for overlap-save fast convolution */
    constexpr static uint32_t fft_size {1UL << static_cast<uint32_t>(std::floor(std::log2(block_size + ir_size - 1)) + 1)};

    arm_rfft_fast_instance_f32 fft;

    std::array<float, 2 * fft_size> ir_fft;
    std::array<float, 2 * fft_size> input_fft;
    std::array<float, fft_size> input;
};

}

#endif /* AUDIO_DSP_HPP_ */
