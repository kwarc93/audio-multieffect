/*
 * audio_wm8994ecs.cpp
 *
 *  Created on: 11 sie 2023
 *      Author: kwarc
 */


#include "audio_wm8994ecs.hpp"

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
    int16_t inbuf[128];
    int16_t outbuf[128];
}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

audio_wm8994ecs::audio_wm8994ecs() : sai_drv{audio_sai::id::sai2}
{
    /* Audio driver test */
    static const audio_sai::block::config a_cfg
    {
        audio_sai::block::mode_type::master_tx,
        audio_sai::block::protocol_type::generic,
        audio_sai::block::data_size::_16bit,
        audio_sai::block::sync_type::none,
        audio_sai::block::frame_type::stereo,
        audio_sai::block::audio_freq::_48kHz,
    };

    sai_drv.block_a.configure(a_cfg);

    static const audio_sai::block::config b_cfg
    {
        audio_sai::block::mode_type::slave_rx,
        audio_sai::block::protocol_type::generic,
        audio_sai::block::data_size::_16bit,
        audio_sai::block::sync_type::internal,
        audio_sai::block::frame_type::stereo,
        audio_sai::block::audio_freq::_48kHz,
    };

    sai_drv.block_b.configure(b_cfg);

    sai_drv.loop_read(true);
    sai_drv.read(inbuf, sizeof(inbuf), [](const int16_t *data, std::size_t bytes_read){});

    sai_drv.loop_write(true);
    sai_drv.write(outbuf, sizeof(outbuf), [](std::size_t bytes_written){});
}

audio_wm8994ecs::~audio_wm8994ecs()
{

}

void audio_wm8994ecs::capture(std::vector<audio_input::sample_t> &input, const capture_cb_t &cb)
{
//    sai_drv.loop_read(true);
//    sai_drv.read(inbuf, sizeof(inbuf), [](const int16_t *data, std::size_t bytes_read){});
}

void audio_wm8994ecs::end(void)
{

}


void audio_wm8994ecs::play(const std::vector<audio_output::sample_t> &output, const play_cb_t &cb)
{
//    sai_drv.loop_write(true);
//    sai_drv.write(outbuf, sizeof(outbuf), [](std::size_t bytes_written){});
}

void audio_wm8994ecs::pause(void)
{

}

void audio_wm8994ecs::resume(void)
{

}

void audio_wm8994ecs::stop(void)
{

}

