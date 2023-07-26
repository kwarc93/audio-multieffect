/*
 * gui.cpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#include "gui.hpp"

#include "libs/lvgl/lvgl.h"
#include "libs/lvgl/demos/lv_demos.h"

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

lv_disp_drv_t lvgl_disp_drv;
lv_disp_draw_buf_t lvgl_draw_buf;

#if HAL_LCD_USE_DOUBLE_FRAMEBUF == 0
__attribute__((section(".dtcmram"))) static lv_color_t lvgl_buf[64 * 1024 / sizeof(lv_color_t)];
#endif

void lvgl_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    using display_t = hal::displays::primary;
    display_t *display = static_cast<display_t*>(disp_drv->user_data);

#if HAL_LCD_USE_DOUBLE_FRAMEBUF
    display->set_frame_buffer(reinterpret_cast<display_t::pixel_t*>(color_p));
    lv_disp_flush_ready(disp_drv);
#else
    display->draw_data(area->x1, area->y1, area->x2, area->y2, reinterpret_cast<display_t::pixel_t*>(color_p));
#endif
}

void lvgl_disp_render_start(struct _lv_disp_drv_t * disp_drv)
{
    using display_t = hal::displays::primary;
    display_t *display = static_cast<display_t*>(disp_drv->user_data);
    display->wait_for_vsync();
}

void gui_timer_callback(void *arg)
{
    gui *gui_ao = static_cast<gui*>(arg);

    static const gui::event e { gui::timer_evt_t {}, gui::event::flags::static_storage };
    gui_ao->send(e);
}

}

//-----------------------------------------------------------------------------
/* public */

gui::gui() : active_object("gui", osPriorityNormal, 4096)
{
    lv_init();

#if HAL_LCD_USE_DOUBLE_FRAMEBUF
    lv_disp_draw_buf_init(&lvgl_draw_buf, hal::displays::frame_buffer.data(), hal::displays::frame_buffer2.data(), display.width() * display.height());
#else
    lv_disp_draw_buf_init(&lvgl_draw_buf, lvgl_buf, NULL, sizeof(lvgl_buf) / sizeof(lv_color_t));
#endif

    lv_disp_drv_init(&lvgl_disp_drv);
    lvgl_disp_drv.hor_res = display.width();
    lvgl_disp_drv.ver_res = display.height();
    lvgl_disp_drv.flush_cb = lvgl_disp_flush;
    lvgl_disp_drv.render_start_cb = lvgl_disp_render_start;
    lvgl_disp_drv.user_data = &this->display;
    lvgl_disp_drv.draw_buf = &lvgl_draw_buf;
    lvgl_disp_drv.full_refresh = HAL_LCD_USE_DOUBLE_FRAMEBUF;
    lv_disp_drv_register(&lvgl_disp_drv);

    display.enable_vsync(HAL_LCD_USE_DOUBLE_FRAMEBUF);
    display.set_draw_callback([](){ lv_disp_flush_ready(&lvgl_disp_drv); });
    display.backlight(true);

    this->timer = osTimerNew(gui_timer_callback, osTimerPeriodic, this, NULL);
    assert(this->timer != nullptr);
    osTimerStart(this->timer, 10);
};

//-----------------------------------------------------------------------------
/* private */

void gui::dispatch(const event &e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void gui::event_handler(const timer_evt_t &e)
{
    lv_timer_handler();
}

void gui::event_handler(const demo_test_evt_t &e)
{
    lv_demo_music();
}


