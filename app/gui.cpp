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

#if HAL_LCD_USE_DOUBLE_FRAMEBUF == 0
__attribute__((section(".dtcmram"))) static lv_color_t lvgl_buf[64 * 1024 / sizeof(lv_color_t)];
#endif

void gui_timer_callback(void *arg)
{
    gui *gui_ao = static_cast<gui*>(arg);

    static const gui::event e { gui::timer_evt_t {}, gui::event::flags::static_storage };
    gui_ao->send(e);
}

lv_disp_drv_t disp_drv;
lv_disp_draw_buf_t draw_buf;

void gui_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    hal::displays::tft_lcd *display = static_cast<hal::displays::tft_lcd*>(disp_drv->user_data);

#if HAL_LCD_USE_DOUBLE_FRAMEBUF
    lcd->set_framebuf(color_p);
#else
    display->draw_data(area->x1, area->y1, area->x2, area->y2, reinterpret_cast<drivers::lcd::pixel_t*>(color_p));
#endif

    lv_disp_flush_ready(disp_drv);
}

}

//-----------------------------------------------------------------------------
/* public */

gui::gui() : active_object("gui", osPriorityNormal, 4096)
{
    lv_init();

#if HAL_LCD_USE_DOUBLE_FRAMEBUF
        lv_disp_draw_buf_init(&draw_buf, display.get_framebuf_1(), display.get_framebuf_2(), display.width() * display.height());
#else
        lv_disp_draw_buf_init(&draw_buf, lvgl_buf, NULL, sizeof(lvgl_buf) / sizeof(lv_color_t));
#endif

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = display.width();
    disp_drv.ver_res = display.height();
    disp_drv.flush_cb = gui_disp_flush;
    disp_drv.render_start_cb = nullptr;
    disp_drv.user_data = &this->display;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = HAL_LCD_USE_DOUBLE_FRAMEBUF;
    lv_disp_drv_register(&disp_drv);

    display.backlight(true);

    this->timer = osTimerNew(gui_timer_callback, osTimerPeriodic, this, NULL);
    assert(this->timer != nullptr);
    osTimerStart(this->timer, 5);
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
    lv_demo_benchmark();
}


