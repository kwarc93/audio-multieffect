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
    enum class id { sai1, sai2 };

    sai_base(id id);
    virtual ~sai_base();

    void sync_with(id id);

    class block
    {
    public:
        enum class id { a, b };
        enum class mode_type { master, slave };
        enum class protocol_type { i2s, pcm, tdm, ac97 };
        enum class data_size { _8bit, _10bit, _16bit, _20bit, _24bit, _32bit };
        enum class frame_type { stereo, mono };

        struct config
        {
            uint32_t frequency;
            mode_type mode;
            protocol_type protocol;
            data_size data;
            frame_type frame;
        };

        explicit block(id id);
        void configure(const config &cfg);
        void sync_with(id id);
    private:
        struct block_hw;
        block_hw &hw;
    };

    block block_a;
    block block_b;

private:
    struct base_hw;
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
