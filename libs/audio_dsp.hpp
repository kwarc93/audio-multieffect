/*
 * audio_dsp.hpp
 *
 *  Created on: 6 wrz 2023
 *      Author: kwarc
 */

#ifndef AUDIO_DSP_HPP_
#define AUDIO_DSP_HPP_

#include <cstdint>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <array>
#include <vector>
#include <random>

/* CMSIS DSP library */
#include <cmsis/cmsis_device.h>
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
    enum class shape {sawtooth, square, triangle, sine, cosine, noise};

    oscillator(shape shape, float freq, uint32_t fs) : fs{fs}
    {
        this->wave_shape = shape;
        this->frequency = freq;

        this->counter = 0;
        this->counter_limit = this->fs / this->frequency;
    }

    void set_frequency(float f)
    {
        if (this->frequency == f || f <= 0)
            return;

        this->frequency = f;
        this->counter_limit = this->fs / this->frequency;
    }

    void set_shape(shape s)
    {
        if (this->wave_shape == s)
            return;

        this->wave_shape = s;
    }

    float generate(void)
    {
        /* Unipolar sawtooth counter */
        this->counter %= this->counter_limit;
        float out = static_cast<float>(this->counter++) / this->counter_limit;

        switch (this->wave_shape)
        {
            case shape::sawtooth:
                out = 2 * (out - 0.5f);
                break;
            case shape::square:
                out = (2 * (out - 0.5f) >= 0) ? 1 : -1;
                break;
            case shape::triangle:
                out = -4 * std::abs(out - 0.5f) + 1;
                break;
            case shape::sine:
                out = arm_sin_f32(-2 * pi * out + pi);
                break;
            case shape::cosine:
                out = arm_cos_f32(-2 * pi * out + pi);
                break;
            case shape::noise:
                /* TODO */
                break;
            default:
                break;
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

class delay_line
{
public:

    delay_line(float max_delay, uint32_t fs) : fs{fs}, allocated{true}
    {
        this->memory_length = this->delay = std::ceil(fs * max_delay);
        this->memory = new float [this->memory_length];
        this->write_idx = 0;
        this->frac = 0;
        this->aph = 0;

        memset(this->memory, 0, sizeof(float) * this->memory_length);
    }

    delay_line(float *samples_memory, uint32_t memory_length, uint32_t fs) : fs{fs}, allocated{false}
    {
        this->memory_length = this->delay = memory_length;
        this->memory = samples_memory;
        this->write_idx = 0;
        this->frac = 0;
        this->aph = 0;

        memset(this->memory, 0, sizeof(float) * this->memory_length);
    }

    ~delay_line()
    {
        if (this->allocated)
            delete [] this->memory;
    }

    void set_delay(float d)
    {
        d *= this->fs;
        if (d < 1)
        {
            /* Zero delay is not allowed */
            this->delay = 1;
            this->frac = 0;
        }
        else
        {
            this->delay = d;
            this->frac = d - this->delay;
        }
    }

    template<bool interpolate = false>
    float get(void)
    {
        const float s0 = this->at(this->delay);
        if constexpr (!interpolate) return s0;

        /* Allpass interpolation */
        const float s1 = this->at(this->delay + 1);
        const float d = (1 - this->frac) / (1 + this->frac); // Or just '(1 - this->frac)'
        return this->aph = s1 + d * (s0 - this->aph);
    }

    float at(uint32_t d)
    {
        int32_t read_idx = this->write_idx - d;
        if (read_idx < 0)
            read_idx += this->memory_length;

        return this->memory[read_idx];
    }

    void put(float sample)
    {
        this->memory[this->write_idx++] = sample;
        if (this->write_idx == this->memory_length)
            this->write_idx = 0;
    }
private:
    const uint32_t fs;
    const bool allocated;

    float *memory;
    uint32_t memory_length;
    uint32_t write_idx;
    uint32_t delay;
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

template<uint8_t factor, uint32_t block_size>
class interpolator
{
public:
    interpolator()
    {
        arm_fir_interpolate_init_f32
        (
            &this->instance,
            factor,
            this->fir_taps,
            const_cast<float*>((factor == 2) ? this->intrpl_x2_fir_coeffs.data() : this->intrpl_x4_fir_coeffs.data()),
            this->state.data(),
            block_size
        );
    }

    void process(const float *in, float *out)
    {
        arm_fir_interpolate_f32(&this->instance, const_cast<float*>(in), out, block_size);
    }

private:
    constexpr static std::array<float, 16> intrpl_x2_fir_coeffs
    {
        -0.0067775138306452,0.0000000000008532,0.0394577742308829,-0.0000000000019968,
        -0.1426580934283212,0.0000000000031691,0.6098363606616076,0.9999999999963272,
        0.6098363606616076,0.0000000000031691,-0.1426580934283212,-0.0000000000019968,
        0.0394577742308829,0.0000000000008532,-0.0067775138306452,-0.0000000000001965
    };

    constexpr static std::array<float, 32> intrpl_x4_fir_coeffs
    {
        -0.0045593193913062,-0.0067775138306635,-0.0051777598602224,-0.0000000000022829,
        0.0257844009649526,0.0394577742309272,0.0311866090727734,0.0000000000053353,
        -0.0877010984438216,-0.1426580934283734,-0.1220465283940888,-0.0000000000084620,
        0.2910057852329270,0.6098363606616314,0.8713054198327470,1.0000000000098053,
        0.8713054198327470,0.6098363606616314,0.2910057852329270,-0.0000000000084620,
        -0.1220465283940888,-0.1426580934283734,-0.0877010984438216,0.0000000000053353,
        0.0311866090727734,0.0394577742309272,0.0257844009649526,-0.0000000000022829,
        -0.0051777598602224,-0.0067775138306635,-0.0045593193913062,0.0000000000005274
    };

    constexpr static uint32_t fir_taps = (factor == 2) ? intrpl_x2_fir_coeffs.size() : intrpl_x4_fir_coeffs.size();

    arm_fir_interpolate_instance_f32 instance;
    std::array<float, (fir_taps / factor) + block_size - 1> state;

    static_assert(factor == 2 || factor == 4);
    static_assert(fir_taps % factor == 0);
};

template<uint8_t factor, uint32_t block_size>
class decimator
{
public:
    decimator()
    {
        arm_fir_decimate_init_f32
        (
            &this->instance,
            fir_taps,
            factor,
            const_cast<float*>((factor == 2) ? this->decim_x2_fir_coeffs.data() : this->decim_x4_fir_coeffs.data()),
            this->state.data(),
            block_size
        );
    }

    void process(const float *in, float *out)
    {
        arm_fir_decimate_f32(&this->instance, const_cast<float*>(in), out, block_size);
    }

private:
    constexpr static std::array<float, 31> decim_x2_fir_coeffs
    {
        -0.0017003969036736,0.0000000000000000,0.0029373315708907,-0.0000000000000000,
        -0.0067300913664044,0.0000000000000000,0.0140938879039919,-0.0000000000000000,
        -0.0267850358200538,0.0000000000000000,0.0490989605935754,-0.0000000000000000,
        -0.0969383327763008,0.0000000000000000,0.3156195633244823,0.5008082269469846,
        0.3156195633244823,0.0000000000000000,-0.0969383327763008,-0.0000000000000000,
        0.0490989605935754,0.0000000000000000,-0.0267850358200538,-0.0000000000000000,
        0.0140938879039919,0.0000000000000000,-0.0067300913664044,-0.0000000000000000,
        0.0029373315708907,0.0000000000000000,-0.0017003969036736
    };
    constexpr static std::array<float, 31> decim_x4_fir_coeffs
    {
        -0.0012038799983330,-0.0020533609372476,-0.0020796290083966,0.0000000000000000,
        0.0047649006919877,0.0098960336352151,0.0099784642689638,-0.0000000000000000,
        -0.0189637894592323,-0.0362933212113035,-0.0347620349518664,0.0000000000000000,
        0.0686322820566259,0.1532657643231546,0.2234584634611206,0.2507202142586236,
        0.2234584634611206,0.1532657643231546,0.0686322820566259,0.0000000000000000,
        -0.0347620349518664,-0.0362933212113035,-0.0189637894592323,-0.0000000000000000,
        0.0099784642689638,0.0098960336352151,0.0047649006919877,0.0000000000000000,
        -0.0020796290083966,-0.0020533609372476,-0.0012038799983330
    };
    constexpr static uint32_t fir_taps = (factor == 2) ? decim_x2_fir_coeffs.size() : decim_x4_fir_coeffs.size();

    arm_fir_decimate_instance_f32 instance;
    std::array<float, fir_taps + block_size - 1> state;

    static_assert(factor == 2 || factor == 4);
};

//-----------------------------------------------------------------------------

/* Exponential moving average filter */
class averaging_filter
{
public:
    averaging_filter(float time_constant, float time_delta, float inital_value = 0)
    {
        this->alpha1 = 1 - std::exp(-time_delta / time_constant);
        this->alpha2 = 1 - this->alpha1;
        this->output = inital_value;
    }
    virtual ~averaging_filter() = default;

    float process(float input)
    {
        return output = this->alpha2 * output + this->alpha1 * input;
    }

    void reset(float value)
    {
        this->output = value;
    }
private:
    float alpha1, alpha2;
    float output;
};

//-----------------------------------------------------------------------------

/* Short 3-point median filter */
class median_filter
{
public:
    median_filter(float median = 0.0f) : median{median}, x2{median}, x3{median} {};

    float process(float x1)
    {
        median = std::max(std::min(x1, x2), std::min(std::max(x1, x2), x3));
        x3 = x2;
        x2 = x1;
        return median;
    }

private:
    float median, x2, x3;
};

//-----------------------------------------------------------------------------

enum class basic_iir_type {allpass, lowpass, highpass};

template<basic_iir_type type>
class basic_iir
{
public:
    basic_iir() : c{0}, h{0} {}

    void calc_coeff(float fc, float fs)
    {
        const float wc = fc / fs;
        const float k = std::tan(pi * wc);
        this->c = (k - 1) / (k + 1);
    }

    void set_coeff(float c)
    {
        this->c = c;
    }

    float process(float x)
    {
        float y = 0;

        /* Allpass */
        const float xh = x - this->c * this->h;
        y = this->c * xh + this->h;
        this->h = xh;

        if constexpr (type == basic_iir_type::lowpass)
        {
            y = 0.5f * (x + y);
        }
        else if constexpr (type == basic_iir_type::highpass)
        {
            y = 0.5f * (x - y);
        }

        return y;
    }
private:
    float c;
    float h;
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

//-----------------------------------------------------------------------------

/* Universal comb filter with optional lowpass filter, delay tap, delay modulation & interpolation */
class unicomb
{
public:
    unicomb(float bl, float fb, float ff, float delay, uint32_t fs) :
    fs{fs}, bl{bl}, fb{fb}, ff{ff}, delay{delay, fs}
    {
        this->normalize();
    }

    unicomb(float bl, float fb, float ff, float *delay_line_mem, uint32_t del_line_len, uint32_t fs) :
    fs{fs}, bl{bl}, fb{fb}, ff{ff}, delay{delay_line_mem, del_line_len, fs}
    {
        this->normalize();
    }

    void set_blend(float bl)
    {
        this->bl = bl;
    }

    void set_feedback(float fb)
    {
        this->fb = fb;
    }

    void set_feedforward(float ff)
    {
        this->ff = ff;
    }

    void set_delay(float d)
    {
        this->delay.set_delay(d);
    }

    void set_lowpass(float fc)
    {
        this->lowpass.calc_coeff(fc, this->fs);
    }

    float at(uint32_t d)
    {
        return this->delay.at(d);
    }

    void normalize(void)
    {
        /* L2 normalization (in case when unicomb acts as IIR, needs update after coeffs change) */
        if (this->fb > 0 && this->ff == 0)
            this->bl = std::sqrt(1 - this->fb * this->fb);
    }

    template<bool lowpass_enabled = false, bool intrpl_enabled = false, uint32_t delay_tap = 0>
    float process(float in)
    {
        const float del = this->delay.get<intrpl_enabled>();
        float tap = del;
        if constexpr (delay_tap > 0) tap = this->delay.at(delay_tap);
        if constexpr (lowpass_enabled) tap = this->lowpass.process(tap);
        const float h = in + this->fb * tap;
        this->delay.put(h);
        return this->ff * del + this->bl * h;
    }

private:
    const uint32_t fs;

    float bl, fb, ff;
    delay_line delay;
    basic_iir<basic_iir_type::lowpass> lowpass;
};

/* Linear Predictive Coding */
class lpc
{
public:
    lpc()
    {

    }

    void process(const float *x, uint32_t x_len, uint8_t ord, float *a, float *g)
    {
        assert(ord < x_len);

        /* Resize if x length is changed */
        this->autocorr.resize(2 * x_len - 1);

        /* Autocorrelation with lag (ord) */
        arm_correlate_f32(const_cast<float*>(x), x_len, const_cast<float*>(x), x_len, this->autocorr.data());

        float *r = this->autocorr.data() + (x_len - 1);

        /* Norm */
        float norm;
        arm_power_f32(r, ord + 1, &norm);
        arm_sqrt_f32(norm, &norm);

        if (norm != 0)
        {
            arm_levinson_durbin(r, a + 1, g, ord);
            arm_negate_f32(a + 1, a + 1, ord);
        }
        else
        {
            arm_fill_f32(0, a + 1, ord + 1);
        }

        a[0] = 1;

        arm_dot_prod_f32(a, r, ord + 1, g);
        arm_sqrt_f32(*g, g);

    }
private:
    /* Levinson-Durbin algorithm from CMSIS DSP library */
    void arm_levinson_durbin (const float *phi, float *a, float *err, int nbCoefs)
    {
        float e;
        int p;

        a[0] = phi[1] / phi[0];

        e = phi[0] - phi[1] * a[0];
        for (p = 1; p < nbCoefs; p++)
        {
            float suma = 0.0f;
            float sumb = 0.0f;
            float k;
            int nb, j, i;

            for (i = 0; i < p; i++)
            {
                suma += a[i] * phi[p - i];
                sumb += a[i] * phi[i + 1];
            }

            k = (phi[p + 1] - suma) / (phi[0] - sumb);

            nb = p >> 1;
            j = 0;
            for (i = 0; i < nb; i++)
            {
                float x, y;

                x = a[j] - k * a[p - 1 - j];
                y = a[p - 1 - j] - k * a[j];

                a[j] = x;
                a[p - 1 - j] = y;

                j++;
            }

            nb = p & 1;
            if (nb)
            {
                a[j] = a[j] - k * a[p - 1 - j];
            }

            a[p] = k;
            e = e * (1.0f - k * k);

        }
        *err = e;
    }

    std::vector<float> autocorr;
};

/* Pitch detector based on McLeod method */
template<uint16_t block_size, uint16_t window_size>
class pitch_detector
{
public:
    pitch_detector(uint32_t fs) : input{}, nsdf{}, fs{fs}, pitch{-1}, clarity{0}
    {
        arm_rfft_fast_init_f32(&this->fft, 2 * window_size);
        arm_fill_f32(0, this->input.data(), this->input.size());
    }

    ~pitch_detector()
    {

    }

    bool process(const float *in)
    {
        bool pitch_detected = false;

        /* 1. Sliding window of input blocks */
        const uint32_t move_size = this->input.size() - block_size;
        arm_copy_f32(this->input.data() + block_size, this->input.data(), move_size);
        arm_copy_f32(const_cast<float*>(in), this->input.data() + move_size, block_size);

        /* 2. Compute the Normalized Square Difference Function */
        calculate_nsdf_fast(this->input.data(), window_size);

        /* 3. Find the best peak in NSDF & get its tau (lag) and value */
        float tau, val;
        pitch_detected = find_best_peak(&tau, &val, 0.9f);
        if (pitch_detected)
        {
            this->pitch = this->fs / tau;
            this->clarity = val;
        }

        return pitch_detected;
    }

    float get_pitch(void)
    {
        return this->pitch;
    }

    float get_clarity(void)
    {
        return this->clarity;
    }
private:
    void calculate_nsdf(const float *x, std::size_t w)
    {
        /* Standard way of NSDF calculation */
        for (unsigned tau = 0; tau < w; ++tau)
        {
            float num = 0;
            float den = 0;
            for (unsigned j = 0; j < (w - tau); ++j)
            {
                const auto x_j = x[j];
                const auto x_j_tau = x[j+tau];
                num += x_j * x_j_tau;
                den += x_j * x_j + x_j_tau * x_j_tau;
            }

            /* FIXME: Its unlikely but 'den' could be 0 */
            this->nsdf[tau] = 2 * num / den;
        }
    }

    void calculate_nsdf_fast(const float *x, std::size_t w)
    {
        /* Fast way (using FFT) of NSDF calculation */

        /* Step 1: Autocorrelation via FFT */
        /* 1.1 Zero pad the window by the number of NSDF values required (pad=w) */
        arm_fill_f32(0, this->padded_input.data() + w, w); // Can be done once at init
        arm_copy_f32((float*)x, this->padded_input.data(), w);

        /* 1.2 Take a Fast Fourier Transform of this real signal */
        arm_copy_f32(this->padded_input.data(), this->fft_input.data(), this->fft_input.size());
        arm_rfft_fast_f32(&this->fft, this->fft_input.data(), this->fft_output.data(), 0);

        /* 1.3 For each complex coefficient, multiply it by its conjugate (giving the power spectrial density) */
        arm_cmplx_mag_squared_f32(this->fft_output.data(), this->fft_input.data(), w);

        this->fft_output[0] = this->fft_input[0];
        this->fft_output[1] = 0;
        this->padded_input[0] = this->padded_input[0] * this->padded_input[0];
        for (unsigned i = 1; i < w; i++)
        {
            /* Convert real to complex */
            this->fft_output[(2 * i) + 0] = this->fft_input[i];
            this->fft_output[(2 * i) + 1] = 0;

            /* Cumulative sum of squares */
            this->padded_input[i] = this->padded_input[i] * this->padded_input[i] + this->padded_input[i - 1];
        }

        /* 1.4 Take the inverse Fast Fourier Transform */
        arm_rfft_fast_f32(&this->fft, this->fft_output.data(), this->fft_input.data(), 1);

        /* Step 2: Precompute cumulative sum of squares (already done in above loop) */

        /* Step 3: Build NSDF */
        auto &r = this->fft_input;
        auto &s = this->padded_input;

        /* 3.1 Special case for tau = 0 */
        unsigned tau = 0;
        this->nsdf[tau] = 2 * r[tau] / (s[w - 1] + (s[w - 1] - s[tau]));
        for (tau = 1; tau < w; ++tau)
        {
            float mp = s[w - 1 - tau] + (s[w - 1] - s[tau - 1]);
            this->nsdf[tau] = 2 * r[tau] / mp;
        }
    }

    bool find_best_peak(float *tau, float *value, float threshold)
    {
        bool peak_found = false;

        *tau = 0;
        *value = 0;

        const auto w = this->nsdf.size() - 1;

        for (unsigned i = 1; i < w; ++i)
        {
            auto y1 = this->nsdf[i - 1];
            auto y2 = this->nsdf[i];
            auto y3 = this->nsdf[i + 1];

            if (y2 > threshold && y2 > y1 && y2 > y3)
            {
                /* Parabolic interpolation around lag 'i' */
                auto y1 = this->nsdf[i - 1];
                auto y2 = this->nsdf[i];
                auto y3 = this->nsdf[i + 1];

                float den = y1 + y3 - 2 * y2;
                float delta = y1 - y3;
                if (den != 0)
                {
                    *tau = i + delta / (2 * den);
                    *value = y2 - delta * delta / (8 * den);
                }
                else
                {
                    *tau = i;
                    *value = y2;
                }

                peak_found = true;
                break;
            }
        }

        return peak_found;
    }

private:
    /* Max supported FFT size is 4096 */
    static_assert((2 * window_size) <= 4096);

    /* Ceil FFT size to next power of 2 based on 2x window_size */
    constexpr static uint32_t fft_size {1UL << static_cast<uint32_t>(std::floor(std::log2(2 * window_size - 1)) + 1)};

    arm_rfft_fast_instance_f32 fft;
    std::array<float, fft_size> fft_input;
    std::array<float, fft_size> fft_output;
    std::array<float, fft_size> padded_input;
    std::array<float, window_size> input;
    std::array<float, window_size> nsdf;
    const uint32_t fs;
    float pitch;
    float clarity;
};

}

#endif /* AUDIO_DSP_HPP_ */
