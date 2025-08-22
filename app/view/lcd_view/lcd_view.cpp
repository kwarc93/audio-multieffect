/*
 * lcd_view.cpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#include "lcd_view.hpp"

#include "libs/lvgl/lvgl.h"

#include "ui.h"
#include "app/utils.hpp"

#include <string>
#include <middlewares/i2c_manager.hpp>

using namespace mfx;
namespace events = lcd_view_events;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

lv_disp_drv_t lvgl_disp_drv;
lv_indev_drv_t lvgl_indev_drv;
lv_disp_draw_buf_t lvgl_draw_buf;

hal::displays::main::pixel_t *lvgl_ready_fb = NULL;
constexpr uint32_t lvgl_wait_flag = 1 << 0;

}

//-----------------------------------------------------------------------------
/* private */

void lcd_view::dispatch(const event &e)
{
    std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
}

void lcd_view::event_handler(const events::shutdown &e)
{
    this->display.backlight(false);
}

void lcd_view::event_handler(const lcd_view_events::configuration &e)
{
    if (ui_sw_sett_dark_mode)
    {
        ui_set_dark_theme(e.dark_mode);
        e.dark_mode ? lv_obj_add_state(ui_sw_sett_dark_mode, LV_STATE_CHECKED) :
                      lv_obj_clear_state(ui_sw_sett_dark_mode, LV_STATE_CHECKED);
    }

    if (ui_sld_sett_displ_bright)
    {
        lv_slider_set_value(ui_sld_sett_displ_bright, e.display_brightness, LV_ANIM_OFF);
    }

    if (ui_sld_sett_main_in_vol)
    {
        lv_slider_set_value(ui_sld_sett_main_in_vol, e.main_input_vol, LV_ANIM_OFF);
    }

    if (ui_sld_sett_aux_in_vol)
    {
        lv_slider_set_value(ui_sld_sett_aux_in_vol, e.aux_input_vol, LV_ANIM_OFF);
    }

    if (ui_sld_sett_out_vol)
    {
        lv_slider_set_value(ui_sld_sett_out_vol, e.output_vol, LV_ANIM_OFF);
    }

    if (ui_sw_sett_mute_audio)
    {
        e.output_muted ? lv_obj_add_state(ui_sw_sett_mute_audio, LV_STATE_CHECKED) :
                         lv_obj_clear_state(ui_sw_sett_mute_audio, LV_STATE_CHECKED);
    }

    if (ui_sw_sett_route_mic_to_aux)
    {
        e.mic_routed_to_aux ? lv_obj_add_state(ui_sw_sett_route_mic_to_aux, LV_STATE_CHECKED) :
                              lv_obj_clear_state(ui_sw_sett_route_mic_to_aux, LV_STATE_CHECKED);
    }
}

void lcd_view::event_handler(const events::initialize &e)
{
    lv_init();

    if constexpr (display.use_double_framebuf)
    {
        hal::displays::main::pixel_t *fb1 = display.get_frame_buffers().first.data();
        hal::displays::main::pixel_t *fb2 = display.get_frame_buffers().second.data();

        lv_disp_draw_buf_init(&lvgl_draw_buf, fb1, fb2, display.width() * display.height());

        display.set_vsync_callback([this]()
        {
            if (lvgl_ready_fb)
            {
                display.set_frame_buffer(lvgl_ready_fb);
                lv_disp_flush_ready(&lvgl_disp_drv);
                this->set(lvgl_wait_flag);
                lvgl_ready_fb = NULL;
            }
        });
    }
    else
    {
#ifdef STM32F7 // TODO: Make it independent from CPU architecture
        __attribute__((section(".dtcmram"))) static lv_color_t lvgl_buf[64 * 1024 / sizeof(lv_color_t)];
#else
        static lv_color_t lvgl_buf[64 * 1024 / sizeof(lv_color_t)];
#endif
        lv_disp_draw_buf_init(&lvgl_draw_buf, lvgl_buf, NULL, sizeof(lvgl_buf) / sizeof(lv_color_t));

        display.set_draw_callback([this]()
        {
            lv_disp_flush_ready(&lvgl_disp_drv);
            this->set(lvgl_wait_flag);
        });
    }

    lv_disp_drv_init(&lvgl_disp_drv);
    lvgl_disp_drv.user_data = this;
    lvgl_disp_drv.draw_buf = &lvgl_draw_buf;
    lvgl_disp_drv.direct_mode = display.use_double_framebuf;
    lvgl_disp_drv.hor_res = display.width();
    lvgl_disp_drv.ver_res = display.height();
    lvgl_disp_drv.wait_cb = [](lv_disp_drv_t * drv)
    {
        lcd_view *this_ = static_cast<lcd_view*>(drv->user_data);
        this_->wait(lvgl_wait_flag, osWaitForever);
    };
    lvgl_disp_drv.flush_cb = [](lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
    {
        using display_t = hal::displays::main;

        lcd_view *this_ = static_cast<lcd_view*>(drv->user_data);
        display_t *display = &this_->display;

        if constexpr (display_t::use_double_framebuf)
        {
            if (lv_disp_flush_is_last(drv))
                lvgl_ready_fb = reinterpret_cast<display_t::pixel_t*>(color_p);
            else
                lv_disp_flush_ready(drv);
        }
        else
        {
#if defined(STM32H7) && defined(CORE_CM7) // TODO: Make it independent from CPU architecture
            const size_t size = lv_area_get_width(area) * lv_area_get_height(area) * sizeof(lv_color_t);
            SCB_CleanDCache_by_Addr(color_p, size);
#endif
            display->draw_data(area->x1, area->y1, area->x2, area->y2, reinterpret_cast<display_t::pixel_t*>(color_p));
        }
    };
    lv_disp_drv_register(&lvgl_disp_drv);

    lv_indev_drv_init(&lvgl_indev_drv);
    lvgl_indev_drv.user_data = this;
    lvgl_indev_drv.type = LV_INDEV_TYPE_POINTER;
    lvgl_indev_drv.read_cb = [](lv_indev_drv_t * drv, lv_indev_data_t * data)
    {
        using display_t = hal::displays::main;

        auto *this_ = static_cast<lcd_view*>(drv->user_data);
        display_t *display = &this_->display;

        int16_t x,y;
        if (display->get_touch(x, y))
        {
            data->point.x = x;
            data->point.y = y;
            data->state = LV_INDEV_STATE_PRESSED;
        }
        else
        {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    };
    lv_indev_drv_register(&lvgl_indev_drv);

    display.backlight(true);

    this->schedule({events::timer {}}, 10, true);

    ui_init(this);
}

void lcd_view::event_handler(const events::timer &e)
{
    lv_timer_handler();
}

void lcd_view::event_handler(const events::show_splash_screen &e)
{
    ui_splash_screen_init();
    lv_scr_load(ui_splash);
}

void lcd_view::event_handler(const events::show_blank_screen &e)
{
    ui_blank_screen_init();

    /* Don't change screen when settings screen is displayed */
    if (lv_scr_act() == ui_settings)
    {
        lv_obj_del(ui_settings_parent_screen);
        ui_settings_parent_screen = ui_blank;
    }
    else
    {
        lv_scr_load_anim(ui_blank, LV_SCR_LOAD_ANIM_FADE_IN, 250, 0, true);
    }

    this->current_effect = effect_id::_count;
}

void lcd_view::event_handler(const events::show_next_effect_screen &e)
{
    this->change_effect_screen(e.id, LV_SCR_LOAD_ANIM_MOVE_LEFT);
}

void lcd_view::event_handler(const events::show_prev_effect_screen &e)
{
    this->change_effect_screen(e.id, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}

void lcd_view::event_handler(const events::set_effect_attributes &e)
{
    std::visit([this, &e](auto &&specific) { this->set_effect_attr(e.basic, specific); }, e.specific);
}

void lcd_view::event_handler(const lcd_view_events::update_effects_list &e)
{
    if (ui_settings == nullptr)
        return;

    ui_settings_clear_effects_list();

    for (auto id : e.effects)
        ui_settings_update_effects_list(static_cast<std::underlying_type_t<effect_id>>(id));
}

void lcd_view::event_handler(const lcd_view_events::update_presets_list &e)
{
    if (ui_settings == nullptr)
        return;

    ui_settings_clear_presets_list();

    for (auto name : e.presets)
        ui_settings_update_presets_list(name.c_str());
}

void lcd_view::event_handler(const events::update_dsp_load &e)
{
    if (ui_lbl_sett_cpu_load != nullptr)
        lv_label_set_text_fmt(ui_lbl_sett_cpu_load, "DSP load: %u%%", e.load_pct);
}

void lcd_view::set_effect_attr(const effect_attr &basic, const tuner_attr &specific)
{
    if (this->current_effect != effect_id::tuner)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_tuner_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_tuner_bypass, LV_STATE_CHECKED);

    lv_label_set_text_fmt(ui_lbl_tuner_pitch, "%.1fHz", specific.out.pitch);
    lv_label_set_text_fmt(ui_lbl_tuner_cents, "%+dc", specific.out.cents);
    lv_obj_set_x(ui_bar_tuner_cents_indicator, 195 + specific.out.cents * 2);
    lv_label_set_text_fmt(ui_lbl_tuner_note, "%c%s%d", std::toupper(specific.out.note), std::isupper(specific.out.note) ? "#" : "", specific.out.octave);
}

void lcd_view::set_effect_attr(const effect_attr &basic, const tremolo_attr &specific)
{
    if (this->current_effect != effect_id::tremolo)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_trem_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_trem_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_trem_rate, utils::map_range<float>(1, 20, lv_arc_get_min_value(ui_arc_trem_rate), lv_arc_get_max_value(ui_arc_trem_rate), specific.ctrl.rate));
    lv_arc_set_value(ui_arc_trem_depth, utils::map_range<float>(0, 0.5, lv_arc_get_min_value(ui_arc_trem_depth), lv_arc_get_max_value(ui_arc_trem_depth), specific.ctrl.depth));

    if (specific.ctrl.shape == tremolo_attr::controls::shape_type::sine)
    {
        lv_obj_clear_state(ui_sw_trem_shape, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_trem_square, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_trem_sine, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_add_state(ui_sw_trem_shape, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_trem_square, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_trem_sine, LV_STATE_CHECKED);
    }
}

void lcd_view::set_effect_attr(const effect_attr &basic, const echo_attr &specific)
{
    if (this->current_effect != effect_id::echo)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_echo_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_echo_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_echo_blur, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_echo_blur), lv_arc_get_max_value(ui_arc_echo_blur), specific.ctrl.blur));
    lv_arc_set_value(ui_arc_echo_time, utils::map_range<float>(0.05f, 1, lv_arc_get_min_value(ui_arc_echo_time), lv_arc_get_max_value(ui_arc_echo_time), specific.ctrl.time));
    lv_arc_set_value(ui_arc_echo_feedb, utils::map_range<float>(0, 0.9f, lv_arc_get_min_value(ui_arc_echo_feedb), lv_arc_get_max_value(ui_arc_echo_feedb), specific.ctrl.feedback));

    if (specific.ctrl.mode == echo_attr::controls::mode_type::echo)
    {
        lv_obj_clear_state(ui_sw_echo_mode, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_echo_mode_delay, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_echo_mode_echo, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_add_state(ui_sw_echo_mode, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_echo_mode_delay, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_echo_mode_echo, LV_STATE_CHECKED);
    }
}

void lcd_view::set_effect_attr(const effect_attr &basic, const chorus_attr &specific)
{
    if (this->current_effect != effect_id::chorus)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_chorus_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_chorus_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_chorus_depth, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_chorus_depth), lv_arc_get_max_value(ui_arc_chorus_depth), specific.ctrl.depth));
    lv_arc_set_value(ui_arc_chorus_rate, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_chorus_rate), lv_arc_get_max_value(ui_arc_chorus_rate), specific.ctrl.rate));
    lv_arc_set_value(ui_arc_chorus_mix, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_chorus_mix), lv_arc_get_max_value(ui_arc_chorus_mix), specific.ctrl.mix));

    if (specific.ctrl.mode == chorus_attr::controls::mode_type::white)
    {
        lv_obj_clear_state(ui_sw_chorus_mode, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_chorus_mode_deep, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_chorus_mode_white, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_add_state(ui_sw_chorus_mode, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_chorus_mode_deep, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_chorus_mode_white, LV_STATE_CHECKED);
    }
}

void lcd_view::set_effect_attr(const effect_attr &basic, const reverb_attr &specific)
{
    if (this->current_effect != effect_id::reverb)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_reverb_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_reverb_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_reverb_bw, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_reverb_bw), lv_arc_get_max_value(ui_arc_reverb_bw), specific.ctrl.bandwidth));
    lv_arc_set_value(ui_arc_reverb_damp, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_reverb_damp), lv_arc_get_max_value(ui_arc_reverb_damp), specific.ctrl.damping));
    lv_arc_set_value(ui_arc_reverb_decay, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_reverb_decay), lv_arc_get_max_value(ui_arc_reverb_decay), specific.ctrl.decay));

    if (specific.ctrl.mode == reverb_attr::controls::mode_type::plate)
    {
        lv_obj_clear_state(ui_sw_reverb_mode, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_reverb_mode_mod, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_reverb_mode_plate, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_add_state(ui_sw_reverb_mode, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_reverb_mode_mod, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_reverb_mode_plate, LV_STATE_CHECKED);
    }
}

void lcd_view::set_effect_attr(const effect_attr &basic, const overdrive_attr &specific)
{
    if (this->current_effect != effect_id::overdrive)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_od_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_od_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_od_mix, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_od_mix), lv_arc_get_max_value(ui_arc_od_mix), specific.ctrl.mix));
    lv_arc_set_value(ui_arc_od_gain, utils::map_range<float>(1, 200, lv_arc_get_min_value(ui_arc_od_gain), lv_arc_get_max_value(ui_arc_od_gain), specific.ctrl.gain));
    lv_arc_set_value(ui_arc_od_tone, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_od_tone), lv_arc_get_max_value(ui_arc_od_tone), specific.ctrl.high));

    if (specific.ctrl.mode == overdrive_attr::controls::mode_type::soft)
    {
        lv_obj_clear_state(ui_sw_od_mode, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_od_mode_hard, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_od_mode_soft, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_add_state(ui_sw_od_mode, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_od_mode_hard, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_od_mode_soft, LV_STATE_CHECKED);
    }
}

void lcd_view::set_effect_attr(const effect_attr &basic, const cabinet_sim_attr &specific)
{
    if (this->current_effect != effect_id::cabinet_sim)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_cab_sim_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_cab_sim_bypass, LV_STATE_CHECKED);

    std::string options;
    for (const auto &ir : specific.ir_names)
        options += std::string(ir) + "\n";
    options.erase(options.end() - 1);

    lv_roller_set_options(ui_roller_cab_sim_ir, options.c_str(), LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(ui_roller_cab_sim_ir, specific.ctrl.ir_idx, LV_ANIM_OFF);
}

void lcd_view::set_effect_attr(const effect_attr &basic, const vocoder_attr &specific)
{
    if (this->current_effect != effect_id::vocoder)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_voc_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_voc_bypass, LV_STATE_CHECKED);

    if (specific.ctrl.hold)
        lv_obj_add_state(ui_btn_voc_hold, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(ui_btn_voc_hold, LV_STATE_CHECKED);

    std::string options;
    for (const auto &band : specific.bands_list)
    {
        if (band == 0) break;
        options += std::to_string(band) + "\n";
    }
    options.erase(options.end() - 1);

    lv_roller_set_options(ui_roller_voc_bands, options.c_str(), LV_ROLLER_MODE_NORMAL);
    const auto pos = std::distance(specific.bands_list.begin(), std::find(specific.bands_list.begin(), specific.bands_list.end(), specific.ctrl.bands));
    lv_roller_set_selected(ui_roller_voc_bands, pos, LV_ANIM_OFF);

    lv_arc_set_value(ui_arc_voc_clarity, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_voc_clarity), lv_arc_get_max_value(ui_arc_voc_clarity), specific.ctrl.clarity));
    lv_arc_set_value(ui_arc_voc_tone, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_voc_tone), lv_arc_get_max_value(ui_arc_voc_tone), specific.ctrl.tone));

    if (specific.ctrl.mode == vocoder_attr::controls::mode_type::vintage)
    {
        lv_obj_clear_state(ui_sw_voc_mode, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_voc_mode_mod, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_voc_mode_vin, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_add_state(ui_sw_voc_mode, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_voc_mode_mod, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_voc_mode_vin, LV_STATE_CHECKED);
    }
}

void lcd_view::set_effect_attr(const effect_attr &basic, const phaser_attr &specific)
{
    if (this->current_effect != effect_id::phaser)
        return;

    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_pha_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_pha_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_pha_rate, utils::map_range<float>(0.01f, 1.0f, lv_arc_get_min_value(ui_arc_pha_rate), lv_arc_get_max_value(ui_arc_pha_rate), specific.ctrl.rate));
    lv_arc_set_value(ui_arc_pha_depth, utils::map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_pha_depth), lv_arc_get_max_value(ui_arc_pha_depth), specific.ctrl.depth));

    if (specific.ctrl.contour == phaser_attr::controls::contour_mode::off)
    {
        lv_obj_clear_state(ui_sw_pha_contour, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_pha_contour_on, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_pha_contour_off, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_add_state(ui_sw_pha_contour, LV_STATE_CHECKED);
        lv_obj_add_state(ui_lbl_pha_contour_on, LV_STATE_CHECKED);
        lv_obj_clear_state(ui_lbl_pha_contour_off, LV_STATE_CHECKED);
    }
}

void lcd_view::change_effect_screen(effect_id id, int dir)
{
    lv_obj_t *new_screen = nullptr;

    switch (id)
    {
    case effect_id::tuner:
        ui_fx_tuner_screen_init();
        new_screen = ui_fx_tuner;
        break;
    case effect_id::tremolo:
        ui_fx_tremolo_screen_init();
        new_screen = ui_fx_tremolo;
        break;
    case effect_id::echo:
        ui_fx_echo_screen_init();
        new_screen = ui_fx_echo;
        break;
    case effect_id::chorus:
        ui_fx_chorus_screen_init();
        new_screen = ui_fx_chorus;
        break;
    case effect_id::reverb:
        ui_fx_reverb_screen_init();
        new_screen = ui_fx_reverb;
        break;
    case effect_id::overdrive:
        ui_fx_overdrive_screen_init();
        new_screen = ui_fx_overdrive;
        break;
    case effect_id::cabinet_sim:
        ui_fx_cabinet_sim_screen_init();
        new_screen = ui_fx_cabinet_sim;
        break;
    case effect_id::vocoder:
        ui_fx_vocoder_screen_init();
        new_screen = ui_fx_vocoder;
        break;
    case effect_id::phaser:
        ui_fx_phaser_screen_init();
        new_screen = ui_fx_phaser;
        break;
    default:
        return;
    }

    /* Don't change screen when settings screen is displayed */
    if (lv_scr_act() == ui_settings)
    {
        lv_obj_del(ui_settings_parent_screen);
        ui_settings_parent_screen = new_screen;
    }
    else
    {
        lv_scr_load_anim(new_screen, static_cast<lv_scr_load_anim_t>(dir), 250, 0, true);
    }

    this->current_effect = id;
}

//-----------------------------------------------------------------------------
/* public */

lcd_view::lcd_view() : active_object("lcd_view", osPriorityAboveNormal, 4096),
display {middlewares::i2c_managers::main::get_instance()}
{
    this->current_effect = effect_id::_count;
    this->send({events::initialize {}});
};
