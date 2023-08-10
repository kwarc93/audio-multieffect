/*
 * sai.hpp
 *
 *  Created on: 9 sie 2023
 *      Author: kwarc
 */

#ifndef STM32F7_SAI_HPP_
#define STM32F7_SAI_HPP_

#include <hal/hal_interface.hpp>

#include <array>

namespace drivers
{

class sai_base
{
public:
    struct base_hw;

    enum class id { sai1, sai2 };

    sai_base(id id);
    virtual ~sai_base();

    void sync_with(id id);

    class block
    {
    public:
        struct block_hw;

        enum class id { a, b };
        enum class mode_type { master_tx, master_rx, slave_tx, slave_rx };
        enum class protocol_type { generic, spdif, ac97 };
        enum class data_size { _8bit = 2, _10bit, _16bit, _20bit, _24bit, _32bit };
        enum class sync_type { none, internal, external };
        enum class frame_type { stereo, mono };
        enum class audio_freq
        {
            _8kHz = 8000, _11_025kHz = 11025, _16kHz = 16000, _22_05kHz = 22050,
            _44_1kHz = 44100, _48kHz = 48000, _96kHz = 96000, _192kHz = 192000
        };

        struct config
        {
            mode_type mode;
            protocol_type protocol;
            data_size data;
            sync_type sync;
            frame_type frame;
            audio_freq frequency;
        };

        explicit block(id id, sai_base *base);
        void enable(bool state);
        void configure(const config &cfg);
        void configure_dma(void *data, uint16_t data_len, std::size_t data_width, bool circular);
    private:
        const block_hw &hw;
    };

    block block_a;
    block block_b;

    static inline std::array<sai_base*, 2> instance; /* Used for global access (e.g. from interrupt) */

private:
    const base_hw &hw;

};

template<typename T>
class sai : public sai_base, public hal::interface::i2s<T>
{
public:
    using sample_t = T;

    sai(id id);
    ~sai();

//-----------------------------------------------------------------------------
/* These methods are unlikely to be used so do not implement them */
    T read(void) override { return T{}; };
    void write(T byte) override {  };
    std::size_t read(T *data, std::size_t size) override { return 0; };
    std::size_t write(const T *data, std::size_t size) override { return 0; };
    void read(T *data, std::size_t size, const typename hal::interface::i2s<T>::read_cb_t &callback) override {  };
    void write(const T *data, std::size_t size, const typename hal::interface::i2s<T>::write_cb_t &callback) override {  };
//-----------------------------------------------------------------------------

    void transfer(const typename hal::interface::i2s<T>::transfer_desc &transfer,
                  const typename hal::interface::i2s<T>::transfer_cb_t &callback, bool loop) override;

private:

};

}

#endif /* STM32F7_SAI_HPP_ */
