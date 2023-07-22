/*
 * gui.cpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#include "gui.hpp"

#include "libs/lvgl/lvgl.h"
#include "libs/lvgl/demos/lv_demos.h"

#include <hal/hal_sdram.hpp>

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
    hal::lcd_tft *lcd = static_cast<hal::lcd_tft*>(disp_drv->user_data);

#if HAL_LCD_USE_DOUBLE_FRAMEBUF
    lcd->set_framebuf(color_p);
#else
    lv_color_t *curr_fb = static_cast<lv_color_t*>(lcd->get_curr_framebuf());
    const size_t w = lv_area_get_width(area);
    for (lv_coord_t y = area->y1; y <= area->y2 && y < disp_drv->ver_res; y++)
    {
        memcpy(&curr_fb[y * disp_drv->hor_res + area->x1], color_p, w * sizeof(lv_color_t));
        color_p += w;
    }
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
        lv_disp_draw_buf_init(&draw_buf, lcd.get_framebuf_1(), lcd.get_framebuf_2(), lcd.width() * lcd.height());
#else
        lv_disp_draw_buf_init(&draw_buf, lvgl_buf, NULL, sizeof(lvgl_buf) / sizeof(lv_color_t));
#endif

    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = lcd.width();
    disp_drv.ver_res = lcd.height();
    disp_drv.flush_cb = gui_disp_flush;
    disp_drv.user_data = &this->lcd;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = HAL_LCD_USE_DOUBLE_FRAMEBUF;
    lv_disp_drv_register(&disp_drv);

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


