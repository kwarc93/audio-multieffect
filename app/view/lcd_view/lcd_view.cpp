/*
 * lcd_view.cpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#include "lcd_view.hpp"

#include "libs/lvgl/lvgl.h"

#include "ui.h"

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

void lvgl_disp_flush(_lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    using display_t = hal::displays::main;
    display_t *display = static_cast<display_t*>(disp_drv->user_data);

    if constexpr (display_t::use_double_framebuf)
    {
        display->set_frame_buffer(reinterpret_cast<display_t::pixel_t*>(color_p));
        lv_disp_flush_ready(disp_drv);
    }
    else
    {
        display->draw_data(area->x1, area->y1, area->x2, area->y2, reinterpret_cast<display_t::pixel_t*>(color_p));
    }
}

void lvgl_input_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    using display_t = hal::displays::main;
    display_t *display = static_cast<hal::displays::main*>(drv->user_data);

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
}

void lcd_view_timer_callback(void *arg)
{
    lcd_view *lcd_view_ao = static_cast<lcd_view*>(arg);

    static const lcd_view::event e { events::timer{}, lcd_view::event::flags::immutable };
    lcd_view_ao->send(e);
}

template<typename T>
T map_range(T in_start, T in_end, T out_start, T out_end, T in_value)
{
    const T in_range = in_end - in_start;
    const T out_range = out_end - out_start;

    return (in_value - in_start) * out_range / in_range + out_start;
}

}

//-----------------------------------------------------------------------------
/* private */

void lcd_view::dispatch(const event &e)
{
    std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
}

void lcd_view::event_handler(const events::timer &e)
{
    lv_timer_handler();
}

void lcd_view::event_handler(const events::show_splash_screen &e)
{
    ui_splash_screen_init();
    lv_disp_load_scr(ui_splash);
}

void lcd_view::event_handler(const lcd_view_events::show_blank_screen &e)
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

void lcd_view::set_effect_attr(const effect_attr &basic, const tremolo_attr &specific)
{
    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_trem_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_trem_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_trem_rate, map_range<float>(1, 20, lv_arc_get_min_value(ui_arc_trem_rate), lv_arc_get_max_value(ui_arc_trem_rate), specific.ctrl.rate));
    lv_arc_set_value(ui_arc_trem_depth, map_range<float>(0, 0.5, lv_arc_get_min_value(ui_arc_trem_depth), lv_arc_get_max_value(ui_arc_trem_depth), specific.ctrl.depth));

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
    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_echo_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_echo_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_echo_blur, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_echo_blur), lv_arc_get_max_value(ui_arc_echo_blur), specific.ctrl.blur));
    lv_arc_set_value(ui_arc_echo_time, map_range<float>(0.05f, 1, lv_arc_get_min_value(ui_arc_echo_time), lv_arc_get_max_value(ui_arc_echo_time), specific.ctrl.time));
    lv_arc_set_value(ui_arc_echo_feedb, map_range<float>(0, 0.9f, lv_arc_get_min_value(ui_arc_echo_feedb), lv_arc_get_max_value(ui_arc_echo_feedb), specific.ctrl.feedback));

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
    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_chorus_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_chorus_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_chorus_depth, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_chorus_depth), lv_arc_get_max_value(ui_arc_chorus_depth), specific.ctrl.depth));
    lv_arc_set_value(ui_arc_chorus_rate, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_chorus_rate), lv_arc_get_max_value(ui_arc_chorus_rate), specific.ctrl.rate));
    lv_arc_set_value(ui_arc_chorus_mix, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_chorus_mix), lv_arc_get_max_value(ui_arc_chorus_mix), specific.ctrl.mix));

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
    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_reverb_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_reverb_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_reverb_bw, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_reverb_bw), lv_arc_get_max_value(ui_arc_reverb_bw), specific.ctrl.bandwidth));
    lv_arc_set_value(ui_arc_reverb_damp, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_reverb_damp), lv_arc_get_max_value(ui_arc_reverb_damp), specific.ctrl.damping));
    lv_arc_set_value(ui_arc_reverb_decay, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_reverb_decay), lv_arc_get_max_value(ui_arc_reverb_decay), specific.ctrl.decay));

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
    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_od_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_od_bypass, LV_STATE_CHECKED);

    lv_arc_set_value(ui_arc_od_mix, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_od_mix), lv_arc_get_max_value(ui_arc_od_mix), specific.ctrl.mix));
    lv_arc_set_value(ui_arc_od_gain, map_range<float>(1, 200, lv_arc_get_min_value(ui_arc_od_gain), lv_arc_get_max_value(ui_arc_od_gain), specific.ctrl.gain));
    lv_arc_set_value(ui_arc_od_tone, map_range<float>(0, 1, lv_arc_get_min_value(ui_arc_od_tone), lv_arc_get_max_value(ui_arc_od_tone), specific.ctrl.high));

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
    if (basic.bypassed)
        lv_obj_clear_state(ui_btn_cab_sim_bypass, LV_STATE_CHECKED);
    else
        lv_obj_add_state(ui_btn_cab_sim_bypass, LV_STATE_CHECKED);

    std::string options;
    for (const auto &ir : specific.ctrl.ir_names)
        options += std::string(ir) + "\n";
    options.erase(options.end() - 1);

    lv_roller_set_options(ui_roller_cab_sim_ir, options.c_str(), LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(ui_roller_cab_sim_ir, specific.ctrl.ir_idx, LV_ANIM_OFF);
}

void lcd_view::change_effect_screen(effect_id id, int dir)
{
    lv_obj_t *new_screen = nullptr;

    switch (id)
    {
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
}

//-----------------------------------------------------------------------------
/* public */

lcd_view::lcd_view() : active_object("lcd_view", osPriorityNormal, 8192),
display {middlewares::i2c_managers::main::get_instance()}
{
    lv_init();

    if constexpr (display.use_double_framebuf)
    {
        hal::displays::main::pixel_t *fb1 = display.get_frame_buffers().first.data();
        hal::displays::main::pixel_t *fb2 = display.get_frame_buffers().second.data();
        lv_disp_draw_buf_init(&lvgl_draw_buf, fb1, fb2, display.width() * display.height());
    }
    else
    {
        __attribute__((section(".dtcmram"))) static lv_color_t lvgl_buf[64 * 1024 / sizeof(lv_color_t)];
        lv_disp_draw_buf_init(&lvgl_draw_buf, lvgl_buf, NULL, sizeof(lvgl_buf) / sizeof(lv_color_t));
    }

    lv_disp_drv_init(&lvgl_disp_drv);
    lvgl_disp_drv.hor_res = display.width();
    lvgl_disp_drv.ver_res = display.height();
    lvgl_disp_drv.flush_cb = lvgl_disp_flush;
    lvgl_disp_drv.user_data = &this->display;
    lvgl_disp_drv.draw_buf = &lvgl_draw_buf;
    lvgl_disp_drv.full_refresh = display.use_double_framebuf;
    lv_disp_drv_register(&lvgl_disp_drv);

    lv_indev_drv_init(&lvgl_indev_drv);
    lvgl_indev_drv.type = LV_INDEV_TYPE_POINTER;
    lvgl_indev_drv.read_cb = lvgl_input_read;
    lvgl_indev_drv.user_data = &this->display;
    lv_indev_drv_register(&lvgl_indev_drv);

    display.vsync(display.use_double_framebuf);
    display.set_draw_callback([](){ lv_disp_flush_ready(&lvgl_disp_drv); });
    display.backlight(true);

    this->timer = osTimerNew(lcd_view_timer_callback, osTimerPeriodic, this, NULL);
    assert(this->timer != nullptr);
    osTimerStart(this->timer, 10);

    ui_init(this);
};
