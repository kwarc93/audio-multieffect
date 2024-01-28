/*
 * vocoder.hpp
 *
 *  Created on: 24 sty 2024
 *      Author: kwarc
 */

#ifndef MODEL_VOCODER_VOCODER_HPP_
#define MODEL_VOCODER_VOCODER_HPP_

#include "app/model/effect_interface.hpp"

#include <array>

#include <libs/audio_dsp.hpp>

namespace mfx
{

class vocoder : public effect
{
public:
    vocoder (float clarity = 0.5f, bool hold = false);
    virtual ~vocoder ();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_clarity(float clarity);
    void hold(bool state);

private:
    class filter_bank
    {
    public:
        filter_bank()
        {
            for (unsigned i = 0; i < this->bands; i++)
                this->bandpass_filters[i].set_coeffs(this->bandpass_coeffs[i]);

            for (auto &&f : this->lowpass_filters)
                f.set_coeffs(this->lowpass_coeffs);
        }

        void bandpass(uint32_t band, const float *in, float *out, uint32_t samples)
        {
            this->bandpass_filters[band].process(in, out, samples);
        }

        void lowpass(uint32_t band, const float *in, float *out, uint32_t samples)
        {
            this->lowpass_filters[band].process(in, out, samples);
        }

        static constexpr uint32_t bands {16};

    private:
        /* Lowpass filter (RMS envelope) */
        static constexpr std::array<float, 5> lowpass_coeffs { 0, 0, 1, 1.98f, -0.9801f};

        /* Band: 1: 50 - 159[Hz] */
        static constexpr std::array<float, 10> bandpass_1_coeffs
        {
            0.0000253908432510,0.0000507816865021,0.0000253908432510,1.9930766231408228,-0.9934212036678660,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9973489469544987,-0.9974025505379704
        };
        /* Band: 2: 159 - 200.327[Hz] */
        static constexpr std::array<float, 10> bandpass_2_coeffs
        {
            0.0000036604848021,0.0000073209696041,0.0000036604848021,1.9974483728787567,-0.9981010284516609,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9979577333868821,-0.9984132970629739
        };
        /* Band: 3: 200.327 - 252.397[Hz] */
        static constexpr std::array<float, 10> bandpass_3_coeffs
        {
            0.0000058080276524,0.0000116160553049,0.0000058080276524,1.9965723217555342,-0.9976080598208837,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9972782757110359,-0.9980012730977980
        };
        /* Band: 4: 252.397 - 318[Hz] */
        static constexpr std::array<float, 10> bandpass_4_coeffs
        {
            0.0000092144145305,0.0000184288290610,0.0000092144145305,1.9953437891741053,-0.9969873281821122,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9963350296156561,-0.9974823786303491
        };
        /* Band: 5: 318 - 400.655[Hz] */
        static constexpr std::array<float, 10> bandpass_5_coeffs
        {
            0.0000146164742441,0.0000292329484881,0.0000146164742441,1.9935981231422533,-0.9962058517209192,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9950083517822803,-0.9968289589364828
        };
        /* Band: 6: 400.655 - 504.794[Hz] */
        static constexpr std::array<float, 10> bandpass_6_coeffs
        {
            0.0000231812433704,0.0000463624867408,0.0000231812433704,1.9910852656256894,-0.9952222181356506,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9931176448081189,-0.9960062356874733
        };
        /* Band: 7: 504.794 - 636[Hz] */
        static constexpr std::array<float, 10> bandpass_7_coeffs
        {
            0.0000367560816023,0.0000735121632046,0.0000367560816023,1.9874228634263347,-0.9939844859748729,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9903881474023759,-0.9949704853571905
        };
        /* Band: 8: 636 - 801.31[Hz] */
        static constexpr std::array<float, 10> bandpass_8_coeffs
        {
            0.0000582631419384,0.0001165262838768,0.0000582631419384,1.9820230986587217,-0.9924276038776757,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9863990979473878,-0.9936667539469665
        };
        /* Band: 9: 801.31 - 1009.59[Hz] */
        static constexpr std::array<float, 10> bandpass_9_coeffs
        {
            0.0000923204661870,0.0001846409323741,0.0000923204661870,1.9739786474086038,-0.9904702685411013,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9805028189210709,-0.9920259854785439
        };
        /* Band: 10: 1009.59 - 1272[Hz] */
        static constexpr std::array<float, 10> bandpass_10_coeffs
        {
            0.0001462178381960,0.0002924356763920,0.0001462178381960,1.9618855016501633,-0.9880111575615337,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9716985971374403,-0.9899614008938247
        };
        /* Band: 11: 1272 - 1602.62[Hz] */
        static constexpr std::array<float, 10> bandpass_11_coeffs
        {
            0.0002314459555137,0.0004628919110275,0.0002314459555137,1.9435693549545308,-0.9849245224883534,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9584368118625362,-0.9873638994453899
        };
        /* Band: 12: 1602.62 - 2019.17[Hz] */
        static constexpr std::array<float, 10> bandpass_12_coeffs
        {
            0.0003660846695791,0.0007321693391582,0.0003660846695791,1.9156674320547964,-0.9810552459278270,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9383167623387507,-0.9840961441685229
        };
        /* Band: 13: 2019.17 - 2544[Hz] */
        static constexpr std::array<float, 10> bandpass_13_coeffs
        {
            0.0005785173244832,0.0011570346489664,0.0005785173244832,1.8730004803740419,-0.9762137248213886,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.9076258509221240,-0.9799847896282214
        };
        /* Band: 14: 2544 - 3205.24[Hz] */
        static constexpr std::array<float, 10> bandpass_14_coeffs
        {
            0.0009131773564504,0.0018263547129008,0.0009131773564504,1.8076573959746691,-0.9701714833440057,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.8606504612657724,-0.9748099114933489
        };
        /* Band: 15: 3205.24 - 4038.35[Hz] */
        static constexpr std::array<float, 10> bandpass_15_coeffs
        {
            0.0014393812914308,0.0028787625828615,0.0014393812914308,1.7077305429645615,-0.9626595461894795,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.7886798312872059,-0.9682898711273570
        };
        /* Band: 16: 4038.35 - 5088[Hz] */
        static constexpr std::array<float, 10> bandpass_16_coeffs
        {
            0.0022647967131630,0.0045295934263260,0.0022647967131630,1.5557447797284123,-0.9533739999571791,
            1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.6786533451550325,-0.9600580330493774
        };

        static constexpr std::array<std::array<float, 10>, bands> bandpass_coeffs
        {
            bandpass_1_coeffs,
            bandpass_2_coeffs,
            bandpass_3_coeffs,
            bandpass_4_coeffs,
            bandpass_5_coeffs,
            bandpass_6_coeffs,
            bandpass_7_coeffs,
            bandpass_8_coeffs,
            bandpass_9_coeffs,
            bandpass_10_coeffs,
            bandpass_11_coeffs,
            bandpass_12_coeffs,
            bandpass_13_coeffs,
            bandpass_14_coeffs,
            bandpass_15_coeffs,
            bandpass_16_coeffs,
        };

        std::array<libs::adsp::iir_biquad<2>, bands> bandpass_filters;
        std::array<libs::adsp::iir_biquad<1>, bands> lowpass_filters;
    };

    /* Lowpass filter (RMS envelope) */
    static constexpr std::array<float, 5> lowpass_coeffs { 0, 0, 1, 1.98f, -0.9801f};

    /* Highpass filter, fc = 5088Hz*/
    static constexpr std::array<float, 10> highpass_coeffs
    {
        0.2529996995574256,-0.5059993991148511,0.2529996995574256,0.3334617277963257,-0.3786640264144165,
        1.0000000000000000,-2.0000000000000000,1.0000000000000000,1.4486195475185251,-0.8910563941509072
    };

    libs::adsp::iir_biquad<2> highpass_filter;
    std::array<float, filter_bank::bands> modulator_hold;
    std::array<float, mfx::config::dsp_vector_size> carrier_env_buf, carrier_bp_buf, modulator_env_buf;
    filter_bank carrier_fb, modulator_fb;

    vocoder_attr attr;
};

}

#endif /* MODEL_VOCODER_VOCODER_HPP_ */
