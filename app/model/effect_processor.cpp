/*
 * effect_processor.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "effect_processor.hpp"

#include <cstring>
#include <cmath>
#include <functional>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>

#include <hal_system.hpp>

#include <middlewares/i2c_manager.hpp>

#include <cmsis_device.h> // For managing D-Cache & I-Cache

#include "app/model/tuner/tuner.hpp"
#include "app/model/tremolo/tremolo.hpp"
#include "app/model/echo/echo.hpp"
#include "app/model/chorus/chorus.hpp"
#include "app/model/reverb/reverb.hpp"
#include "app/model/overdrive/overdrive.hpp"
#include "app/model/cabinet_sim/cabinet_sim.hpp"
#include "app/model/vocoder/vocoder.hpp"
#include "app/model/phaser/phaser.hpp"
#include "app/model/amp_sim/amp_sim.hpp"

#include "tusb.h"

using namespace mfx;
namespace events = effect_processor_events;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
    // TUSB TEST
    // Audio controls
    // Current states
    bool mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];      // +1 for master channel 0
    uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];// +1 for master channel 0
    uint32_t sampFreq;
    uint8_t clkValid;
    bool ready;

    // Range states
    audio_control_range_2_n_t(1) volumeRng[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];// Volume range state
    audio_control_range_4_n_t(1) sampleFreqRng;                                    // Sample frequency range state

    // Audio test data
    int16_t my_audio_buffer[128];
    uint16_t test_buffer_audio[CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE / 1000 * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX / 2];
    uint16_t startVal = 0;

    void board_init(void)
    {
        // Enable low-level USBFS stuff

        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin10 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // ID
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin11 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // D-
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin12 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // D+

        drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB2, OTGFS), true);

        NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 5+1, 0 ));

#if OTG_FS_VBUS_SENSE
        /* Configure VBUS Pin */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // Enable VBUS sense (B device) via pin PA9
        USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN;
#else
        // Disable VBUS sense (B device) via pin PA9
        USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

        // B-peripheral session valid override enable
        USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
#endif // vbus sense
    }

    uint32_t cpu_cycles_to_us(uint32_t start, uint32_t end)
    {
        constexpr uint32_t cycles_per_us = hal::system::system_clock / 1000000ul;
        uint32_t total_cycles = end - start;
        return total_cycles / cycles_per_us;
    }
}

//-----------------------------------------------------------------------------
/* private */

void effect_processor::dispatch(const event &e)
{
    std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
}

void effect_processor::event_handler(const events::initialize &e)
{
    /* DSP buffers contain only one channel */
    this->dsp_main_input.resize(config::dsp_vector_size);
    this->dsp_aux_input.resize(config::dsp_vector_size);
    this->dsp_output.resize(config::dsp_vector_size);
}

void effect_processor::event_handler(const events::shutdown &e)
{
    this->audio.mute(true);
    this->audio.stop();
    this->audio.stop_capture();
}

void effect_processor::event_handler(const events::configuration &e)
{
    this->audio.set_input_volume(e.main_input_vol, 0);
    this->audio.set_input_volume(e.aux_input_vol, 1);
    this->audio.set_output_volume(e.output_vol);
    this->audio.route_onboard_mic_to_aux(e.mic_routed_to_aux);
    this->audio.mute(e.output_muted);
}

void effect_processor::event_handler(const events::start_audio &e)
{
    /* Start audio capture */
    this->audio.capture(this->audio_input.buffer.data(), this->audio_input.buffer.size(),
    [this](auto && ...params)
    {
        this->audio_capture_cb(params...);
    },
    true);

    /* Start audio playback */
    this->audio.play(this->audio_output.buffer.data(), this->audio_output.buffer.size(),
    [this](auto && ...params)
    {
        this->audio_play_cb(params...);
    },
    true);
}

void effect_processor::event_handler(const events::add_effect &e)
{
    /* Don't allow duplicates */
    if (!this->find_effect(e.id))
        this->effects.push_back(std::move(this->create_new(e.id)));
}

void effect_processor::event_handler(const events::remove_effect &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
        this->effects.erase(it);
}

void effect_processor::event_handler(const events::move_effect &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        if (it == this->effects.begin() && e.step < 0)
            return;

        if (it == (this->effects.end() - 1) && e.step > 0)
            return;

        /* Only moves by +1/-1 are supported */
        std::swap(*it, *std::next(it, std::clamp(e.step, -1L, 1L)));
    }
}

void effect_processor::event_handler(const events::bypass_effect &e)
{
    auto effect = this->find_effect(e.id);

    if (effect)
        effect->bypass(e.bypassed);

}

void effect_processor::event_handler(const events::set_input_volume &e)
{
    this->audio.set_input_volume(e.main_input_vol, 0);
    this->audio.set_input_volume(e.aux_input_vol, 1);
}

void effect_processor::event_handler(const events::set_output_volume &e)
{
    this->audio.set_output_volume(e.output_vol);
}

void effect_processor::event_handler(const events::route_mic_to_aux &e)
{
    this->audio.route_onboard_mic_to_aux(e.value);

    /* Re-start audio capture */
    this->audio.capture(this->audio_input.buffer.data(), this->audio_input.buffer.size(),
    [this](auto && ...params)
    {
        this->audio_capture_cb(params...);
    },
    true);
}

void effect_processor::event_handler(const events::set_mute &e)
{
    this->audio.mute(e.value);
}

void effect_processor::event_handler(const events::process_audio &e)
{
    const uint32_t cycles_start = hal::system::clock::cycles();

    effect::dsp_input *current_input {&this->dsp_main_input};
    effect::dsp_output *current_output {&this->dsp_output};

    // TUSB TEST (expects 48 samples every 1ms)
    if (ready)
    {
        tud_audio_write((uint8_t *) &my_audio_buffer[0], 2*48);
        tud_audio_write((uint8_t *) &my_audio_buffer[48], 2*48);
        tud_audio_write((uint8_t *) &my_audio_buffer[96], 2*32);
    }

    for (auto &&effect : this->effects)
    {
        if (!effect->is_bypassed())
        {
            effect->set_aux_input(this->dsp_aux_input);
            effect->process(*current_input, *current_output);

            /* Swap current buffers pointers after each effect process so that old output is new input */
            effect::dsp_input *tmp = current_input;
            current_input = current_output;
            current_output = tmp;
        }
    }

    /* Set correct output buffer after all processing */
    current_output = current_input;

    /* Transform normalized DSP samples to RAW buffer (24bit onto 32bit MSB) */
    for (unsigned i = 0; i < current_output->size(); i++)
    {
        decltype(this->audio_output.buffer)::value_type sample;

        constexpr decltype(sample) min = -(1 << (this->audio_input.bps - 1));
        constexpr decltype(sample) max = -min - 1;
        constexpr decltype(sample) scale = -min;

        sample = current_output->at(i) * scale;
        sample = std::clamp(sample, min, max) << 8;

        /* Duplicate left channel to right channel */
        const auto index = this->audio_output.sample_index + 2 * i;
        this->audio_output.buffer[index] = sample;
        this->audio_output.buffer[index + 1] = sample;
    }

#ifdef CORE_CM7
    /* If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
       Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes. */
    SCB_CleanDCache_by_Addr(&this->audio_output.buffer[this->audio_output.sample_index], sizeof(this->audio_output.buffer) / 2);
#endif /* CORE_CM7 */

    const uint32_t cycles_end = hal::system::clock::cycles();
    this->processing_time_us = cpu_cycles_to_us(cycles_start, cycles_end);
}

void effect_processor::event_handler(const events::get_dsp_load &e)
{
    this->notify(events::dsp_load_changed {this->get_processing_load()});
}

void effect_processor::event_handler(const events::set_effect_controls &e)
{
    std::visit([this](auto &&ctrl) { this->set_controls(ctrl); }, e.ctrl);
}

void effect_processor::event_handler(const events::get_effect_attributes &e)
{
    auto effect = this->find_effect(e.id);
    if (effect)
    {
        this->notify_effect_attributes_changed(effect);
    }
}

void effect_processor::event_handler(const effect_processor_events::enumerate_effects_attributes &e)
{
    for (auto it = this->effects.begin(); it != this->effects.end(); ++it)
    {
        bool is_last = std::next(it) == this->effects.end();
        this->notify(events::effect_attributes_enumerated {is_last, (*it)->get_basic_attributes(), (*it)->get_specific_attributes()});
    }
}

void effect_processor::set_controls(const tuner_attr::controls &ctrl)
{
    auto tuner_effect = static_cast<tuner*>(this->find_effect(effect_id::tuner));

    if (tuner_effect == nullptr)
        return;

    tuner_effect->set_a4_tuning(ctrl.a4_tuning);
}

void effect_processor::set_controls(const tremolo_attr::controls &ctrl)
{
    auto tremolo_effect = static_cast<tremolo*>(this->find_effect(effect_id::tremolo));

    if (tremolo_effect == nullptr)
        return;

    tremolo_effect->set_rate(ctrl.rate);
    tremolo_effect->set_depth(ctrl.depth);
    tremolo_effect->set_shape(ctrl.shape);
}

void effect_processor::set_controls(const echo_attr::controls &ctrl)
{
    auto echo_effect = static_cast<echo*>(this->find_effect(effect_id::echo));

    if (echo_effect == nullptr)
        return;

    echo_effect->set_mode(ctrl.mode);
    echo_effect->set_blur(ctrl.blur);
    echo_effect->set_time(ctrl.time);
    echo_effect->set_feedback(ctrl.feedback);
}

void effect_processor::set_controls(const chorus_attr::controls &ctrl)
{
    auto chorus_effect = static_cast<chorus*>(this->find_effect(effect_id::chorus));

    if (chorus_effect == nullptr)
        return;

    chorus_effect->set_depth(ctrl.depth);
    chorus_effect->set_rate(ctrl.rate);
    chorus_effect->set_tone(ctrl.tone);
    chorus_effect->set_mix(ctrl.mix);
    chorus_effect->set_mode(ctrl.mode);
}

void effect_processor::set_controls(const reverb_attr::controls &ctrl)
{
    auto reverb_effect = static_cast<reverb*>(this->find_effect(effect_id::reverb));

    if (reverb_effect == nullptr)
        return;

    reverb_effect->set_bandwidth(ctrl.bandwidth);
    reverb_effect->set_damping(ctrl.damping);
    reverb_effect->set_decay(ctrl.decay);
    reverb_effect->set_mode(ctrl.mode);
}

void effect_processor::set_controls(const overdrive_attr::controls &ctrl)
{
    auto overdrive_effect = static_cast<overdrive*>(this->find_effect(effect_id::overdrive));

    if (overdrive_effect == nullptr)
        return;

    overdrive_effect->set_mode(ctrl.mode);
    overdrive_effect->set_high(ctrl.high);
    overdrive_effect->set_low(ctrl.low);
    overdrive_effect->set_gain(ctrl.gain);
    overdrive_effect->set_mix(ctrl.mix);
}

void effect_processor::set_controls(const cabinet_sim_attr::controls &ctrl)
{
    auto cab_sim_effect = static_cast<cabinet_sim*>(this->find_effect(effect_id::cabinet_sim));

    if (cab_sim_effect == nullptr)
        return;

    cab_sim_effect->set_ir(ctrl.ir_idx);
}

void effect_processor::set_controls(const vocoder_attr::controls &ctrl)
{
    auto vocoder_effect = static_cast<vocoder*>(this->find_effect(effect_id::vocoder));

    if (vocoder_effect == nullptr)
        return;

    const auto current_mode = std::get<vocoder_attr>(vocoder_effect->get_specific_attributes()).ctrl.mode;

    vocoder_effect->set_mode(ctrl.mode);
    vocoder_effect->hold(ctrl.hold);
    vocoder_effect->set_tone(ctrl.tone);
    vocoder_effect->set_clarity(ctrl.clarity);
    vocoder_effect->set_bands(ctrl.bands);

    if (current_mode != ctrl.mode)
    {
        /* Notify about change in internal structure of effect */
        this->notify_effect_attributes_changed(vocoder_effect);
    }
}

void effect_processor::set_controls(const phaser_attr::controls &ctrl)
{
    auto phaser_effect = static_cast<phaser*>(this->find_effect(effect_id::phaser));

    if (phaser_effect == nullptr)
        return;

    phaser_effect->set_rate(ctrl.rate);
    phaser_effect->set_depth(ctrl.depth);
    phaser_effect->set_contour(ctrl.contour);
}

void effect_processor::set_controls(const amp_sim_attr::controls &ctrl)
{
    auto amp_sim_effect = static_cast<amp_sim*>(this->find_effect(effect_id::amplifier_sim));

    if (amp_sim_effect == nullptr)
        return;

    amp_sim_effect->set_mode(ctrl.mode);
    amp_sim_effect->set_input(ctrl.input);
    amp_sim_effect->set_drive(ctrl.drive);
    amp_sim_effect->set_compression(ctrl.compression);
    amp_sim_effect->set_tone_stack(ctrl.bass, ctrl.mids, ctrl.treb);
}

void effect_processor::notify_effect_attributes_changed(const effect *e)
{
    this->notify(events::effect_attributes_changed {e->get_basic_attributes(), e->get_specific_attributes()});
}

std::unique_ptr<effect> effect_processor::create_new(effect_id id)
{
    static const std::map<effect_id, std::function<std::unique_ptr<effect>()>> effect_factory =
    {
        { effect_id::tuner,         []() { return std::make_unique<tuner>(); } },
        { effect_id::tremolo,       []() { return std::make_unique<tremolo>(); } },
        { effect_id::echo,          []() { return std::make_unique<echo>(); } },
        { effect_id::chorus,        []() { return std::make_unique<chorus>(); } },
        { effect_id::reverb,        []() { return std::make_unique<reverb>(); } },
        { effect_id::overdrive,     []() { return std::make_unique<overdrive>(); } },
        { effect_id::cabinet_sim,   []() { return std::make_unique<cabinet_sim>(); } },
        { effect_id::vocoder,       []() { return std::make_unique<vocoder>(); } },
        { effect_id::phaser,        []() { return std::make_unique<phaser>(); } },
        { effect_id::amplifier_sim, []() { return std::make_unique<amp_sim>(); } }
    };

    std::unique_ptr<effect> e = effect_factory.at(id)();
    e->set_callback([this](effect* e) { this->notify_effect_attributes_changed(e); });
    return e;
}

bool effect_processor::find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it)
{
    auto effect_it = std::find_if(begin(this->effects), end(this->effects),
                                  [id](auto &&effect) { return effect->get_basic_attributes().id == id; });

    it = effect_it;

    return effect_it != std::end(this->effects);
}

effect* effect_processor::find_effect(effect_id id)
{
    std::vector<std::unique_ptr<effect>>::iterator it;
    return this->find_effect(id, it) ? (*it).get() : nullptr;
}

void effect_processor::audio_capture_cb(const hal::audio_devices::codec::input_sample_t *input, uint16_t length)
{
    /* WARNING: This method could have been called from interrupt */

#ifdef CORE_CM7
    /* If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
       Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes. */
    SCB_InvalidateDCache_by_Addr(const_cast<hal::audio_devices::codec::input_sample_t*>(input), length * sizeof(*input));
#endif /* CORE_CM7 */

    /* Set current read index for input buffer (double buffering) */
    if (input == &this->audio_input.buffer[0])
        this->audio_input.sample_index = 0;
    else
        this->audio_input.sample_index = this->audio_input.buffer.size() / 2;

    /* Transform RAW samples (24bit extended onto 32bit MSB) to normalized DSP buffer */
    for (unsigned i = this->audio_input.sample_index, j = 0; i < this->audio_input.sample_index + this->audio_input.buffer.size() / 2; i+=2, j++)
    {
        constexpr float scale = 1.0f / (1 << (this->audio_input.bps - 1));
        this->dsp_main_input[j] = (this->audio_input.buffer[i] >> 8) * scale; // Left
        this->dsp_aux_input[j] = (this->audio_input.buffer[i + 1] >> 8) * scale; // Right
        my_audio_buffer[j] = this->audio_input.buffer[i + 1] >> 16;
    }

    /* Send event to process data */
    static const event e{ events::process_audio {}, event::immutable };
    this->send(e, 0);
}

void effect_processor::audio_play_cb(uint16_t sample_index)
{
    /* WARNING: This method could have been called from interrupt */

    /* Set current write index for output buffer (double buffering) */
    if (sample_index == this->audio_output.buffer.size())
        this->audio_output.sample_index = this->audio_output.buffer.size() / 2;
    else
        this->audio_output.sample_index = 0;
}

uint8_t effect_processor::get_processing_load(void)
{
    constexpr uint32_t max_processing_time_us = 1e6 * config::dsp_vector_size / config::sampling_frequency_hz;
    return 100 * this->processing_time_us / max_processing_time_us;
}

//-----------------------------------------------------------------------------
/* public */

effect_processor::effect_processor() :
audio{middlewares::i2c_managers::main::get_instance()}
{
    this->processing_time_us = 0;

    this->send({events::initialize {}});

    // TUSB TEST
    board_init();

    // init device stack on configured roothub port
    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO};
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    // Init values
    sampFreq = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
    clkValid = 1;
    ready = false;

    sampleFreqRng.wNumSubRanges = 1;
    sampleFreqRng.subrange[0].bMin = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bMax = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bRes = 0;

    osThreadAttr_t attr {};
    attr.name = "tusb_thread";
    attr.stack_size = 8192;
    attr.priority = osPriorityAboveNormal;
    osThreadNew([](void *arg){ while(1) { tud_task(); }; }, this, &attr);
}

effect_processor::~effect_processor()
{

}

// TUSB TEST
extern "C"
{
//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
  //blink_interval_ms = BLINK_MOUNTED;
    ready = true;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  //blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  //blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  //blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) ep;

  return false;// Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) itf;

  return false;// Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  (void) itf;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // If request is for our feature unit
  if (entityID == 2) {
    switch (ctrlSel) {
      case AUDIO_FU_CTRL_MUTE:
        // Request uses format layout 1
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

        mute[channelNum] = ((audio_control_cur_1_t *) pBuff)->bCur;

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", mute[channelNum], channelNum);
        return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        volume[channelNum] = (uint16_t) ((audio_control_cur_2_t *) pBuff)->bCur;

        TU_LOG2("    Set Volume: %d dB of channel: %u\r\n", volume[channelNum], channelNum);
        return true;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }
  return false;// Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) ep;

  //    return tud_control_xfer(rhport, p_request, &tmp, 1);

  return false;// Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) itf;

  return false;// Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex);           // Since we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  // Input terminal (Microphone input)
  if (entityID == 1) {
    switch (ctrlSel) {
      case AUDIO_TE_CTRL_CONNECTOR: {
        // The terminal connector control only has a get request with only the CUR attribute.
        audio_desc_channel_cluster_t ret;

        // Those are dummy values for now
        ret.bNrChannels = 1;
        ret.bmChannelConfig = (audio_channel_config_t) 0;
        ret.iChannelNames = 0;

        TU_LOG2("    Get terminal connector\r\n");

        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void *) &ret, sizeof(ret));
      } break;

        // Unknown/Unsupported control selector
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Feature unit
  if (entityID == 2) {
    switch (ctrlSel) {
      case AUDIO_FU_CTRL_MUTE:
        // Audio control mute cur parameter block consists of only one byte - we thus can send it right away
        // There does not exist a range parameter block for mute
        TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &mute[channelNum], 1);

      case AUDIO_FU_CTRL_VOLUME:
        switch (p_request->bRequest) {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &volume[channelNum], sizeof(volume[channelNum]));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

            // Copy values - only for testing - better is version below
            audio_control_range_2_n_t(1)
                ret;

            ret.wNumSubRanges = 1;
            ret.subrange[0].bMin = -90;// -90 dB
            ret.subrange[0].bMax = 90; // +90 dB
            ret.subrange[0].bRes = 1;  // 1 dB steps

            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void *) &ret, sizeof(ret));

            // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
        break;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Clock Source unit
  if (entityID == 4) {
    switch (ctrlSel) {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch (p_request->bRequest) {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Sample Freq.\r\n");
            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &sampFreq, sizeof(sampFreq));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Sample Freq. range\r\n");
            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

            // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
        break;

      case AUDIO_CS_CTRL_CLK_VALID:
        // Only cur attribute exists for this request
        TU_LOG2("    Get Sample Freq. valid\r\n");
        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

      // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  TU_LOG2("  Unsupported entity: %d\r\n", entityID);
  return false;// Yet not implemented
}

bool tud_audio_set_itf_close_ep_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;
  (void) p_request;
  startVal = 0;

  return true;
}
}


