/*
 * hal_interface.hpp
 *
 *  Created on: 21 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_INTERFACE_HPP_
#define HAL_INTERFACE_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>

namespace hal::interface
{
    template<typename T>
    class io_bus
    {
    public:
        typedef std::function<void(const T *data, std::size_t bytes_read)> read_cb_t;
        typedef std::function<void(std::size_t bytes_written)> write_cb_t;

        virtual ~io_bus() {};
        virtual T read(void) = 0;
        virtual void write(T byte) = 0;
        virtual std::size_t read(T *data, std::size_t size) = 0;
        virtual std::size_t write(const T *data, std::size_t size) = 0;
        virtual void read(T *data, std::size_t size, const read_cb_t &callback) = 0;
        virtual void write(const T *data, std::size_t size, const write_cb_t &callback) = 0;
    };

    class serial : public io_bus<std::byte>
    {
    public:
        virtual ~serial() {};
        virtual void listen(bool enable) { this->listening = enable; };
    protected:
        bool listening;
    };

    class i2c : public io_bus<std::byte>
    {
    public:
        virtual ~i2c() {};
        virtual void set_address(uint8_t addr) { this->address = addr; };
        virtual void set_no_stop(bool value) { this->no_stop = value; };
    protected:
        uint8_t address;
        bool no_stop;
    };

    template<typename T>
    class i2s : public io_bus<T>
    {
    public:
        virtual ~i2s() {};
        virtual void loop_read(bool state) { this->read_loop = state; };
        virtual void loop_write(bool state) { this->write_loop = state; };
    protected:
        bool read_loop, write_loop;

    };

    class i2c_device
    {
    public:

        struct transfer_desc
        {
            uint8_t address {0};
            const std::byte *tx_data {nullptr};
            std::size_t tx_size {0};
            std::byte *rx_data {nullptr};
            std::size_t rx_size {0};
            enum class status { ok, error, pending } stat {status::pending};
        };

        typedef std::function<void(const transfer_desc &transfer)> transfer_cb_t;

        i2c_device(i2c &drv) : driver {drv} {};
        virtual ~i2c_device() {};
        virtual void transfer(transfer_desc &descriptor) = 0;
        virtual void transfer(const transfer_desc &descriptor, const transfer_cb_t &callback)
        {
            transfer_desc desc {descriptor};
            this->transfer(desc);
            if (callback)
                callback(desc);
        };
    protected:
        i2c &driver;
    };

    class temperature_sensor
    {
    public:
        virtual ~temperature_sensor() {};
        virtual float read_temperature(void) = 0;
    };

    class led
    {
    public:
        virtual ~led() {};
        virtual void set(uint8_t brightness) = 0;
        virtual uint8_t get(void) = 0;
    };

    class button
    {
    public:
        virtual ~button() {};
        virtual bool is_pressed(void) = 0;
    };

    template<typename T>
    class glcd
    {
    public:
        using pixel_t = T;

        typedef std::function<void(void)> vsync_cb_t;

        virtual ~glcd() {};

        virtual uint16_t width(void) = 0;
        virtual uint16_t height(void) = 0;
        virtual uint8_t bpp(void) = 0;

        virtual void draw_pixel(int16_t x, int16_t y, pixel_t pixel) = 0;
        virtual void draw_data(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_t *data) = 0;

        virtual void enable_vsync(bool state) = 0;
        virtual void set_vsync_callback(const vsync_cb_t &callback) = 0;
    };

    class touch_panel
    {
    public:
        typedef std::function<void(int16_t x, int16_t y)> touch_cb_t;

        touch_panel(const touch_cb_t &callback = {}) : touch_callback {callback} {};
        virtual ~touch_panel() {};

        virtual bool get_touch(int16_t &x, int16_t &y) = 0;
    protected:
        touch_cb_t touch_callback;
    };

    template<typename T>
    class audio_input
    {
    public:
        using sample_t = T;

        typedef std::function<void(const sample_t *input, uint16_t length)> capture_cb_t;

        virtual ~audio_input() {};

        virtual void capture(sample_t *input, uint16_t length, const capture_cb_t &cb, bool loop) = 0;
        virtual void stop_capture(void) = 0;

    protected:
        capture_cb_t capture_callback;
    };

    template<typename T>
    class audio_output
    {
    public:
        using sample_t = T;

        typedef std::function<void(uint16_t output_sample_index)> play_cb_t;

        virtual ~audio_output() {};

        virtual void play(const sample_t *output, uint16_t length, const play_cb_t &cb, bool loop) = 0;
        virtual void pause(void) = 0;
        virtual void resume(void) = 0;
        virtual void stop(void) = 0;
        virtual void set_volume(uint8_t vol) = 0;

    protected:
        play_cb_t play_callback;
    };

    template <typename T, uint16_t S, uint8_t CH, uint8_t B>
    struct audio_buffer
    {
        static constexpr uint16_t samples = S;
        static constexpr uint8_t channels = CH;
        static constexpr uint8_t bps = B;

        volatile uint16_t sample_index;
        alignas(32) std::array<T, samples*channels> buffer; // Alignment at 32-byte boundary needed for DMA & CPU D-Cache
    };

}

#endif /* HAL_INTERFACE_HPP_ */
