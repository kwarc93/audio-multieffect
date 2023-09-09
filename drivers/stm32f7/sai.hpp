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
#include <functional>

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

        enum class dma_evt { transfer_half, transfer_complete, transfer_error, fifo_error };
        typedef std::function<void(dma_evt e)> dma_cb_t;

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
            uint8_t slots;
            uint16_t active_slots;
        };

        explicit block(id id, sai_base *base);
        void enable(bool state);
        void configure(const config &cfg);
        void configure_dma(void *data, uint16_t data_len, std::size_t data_width, const dma_cb_t &cb, bool circular);
        void dma_irq_handler(void);
    private:
        const block_hw &hw;
        dma_cb_t dma_callback;
    };

    static inline std::array<sai_base*, 2> instance; /* Used for global access (e.g. from interrupt) */

private:
    const base_hw &hw;
public:
    block block_a;
    block block_b;
};

template<typename T>
class sai : public sai_base, public hal::interface::i2s<T>
{
public:
    sai(id id) : sai_base {id} {};
    ~sai() {};

//-----------------------------------------------------------------------------
/* These methods are unlikely to be used so do not implement them */
    T read(void) override { return T{}; };
    void write(T byte) override {  };
    std::size_t read(T *data, std::size_t size) override { return 0; };
    std::size_t write(const T *data, std::size_t size) override { return 0; };
//-----------------------------------------------------------------------------

    void read(T *data, std::size_t size, const typename hal::interface::i2s<T>::read_cb_t &callback) override
    {
        this->read_callback = callback;
        this->block_b.configure_dma(data, size / sizeof(*data), sizeof(*data),
                                    [this, data, size](block::dma_evt e)
                                    {
                                        switch (e)
                                        {
                                        case block::dma_evt::transfer_half:
                                            this->read_callback(data, size / 2);
                                            break;
                                        case block::dma_evt::transfer_complete:
                                            this->read_callback(data + (size / sizeof(*data) / 2), size / 2);
                                            break;
                                        case block::dma_evt::transfer_error:
                                        case block::dma_evt::fifo_error:
                                        default:
                                            break;
                                        }
                                    }
                                    ,this->read_loop);
    };

    void write(const T *data, std::size_t size, const typename hal::interface::i2s<T>::write_cb_t &callback) override
    {
        this->write_callback = callback;
        this->block_a.configure_dma((void*)data, size / sizeof(*data), sizeof(*data),
                                    [this, size](block::dma_evt e)
                                    {
                                        switch (e)
                                        {
                                        case block::dma_evt::transfer_half:
                                            this->write_callback(size / 2);
                                            break;
                                        case block::dma_evt::transfer_complete:
                                            this->write_callback(size);
                                            break;
                                        case block::dma_evt::transfer_error:
                                        case block::dma_evt::fifo_error:
                                        default:
                                            break;
                                        }
                                    }
                                    ,this->write_loop);
    };
private:
    typename hal::interface::i2s<T>::read_cb_t read_callback;
    typename hal::interface::i2s<T>::write_cb_t write_callback;
};

}

#endif /* STM32F7_SAI_HPP_ */
