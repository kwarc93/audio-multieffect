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
    if (lv_disp_flush_is_last(disp_drv))
    {
        hal::lcd_tft *lcd = static_cast<hal::lcd_tft*>(disp_drv->user_data);
        lcd->set_framebuf(color_p);

        lv_disp_t *disp = _lv_refr_get_disp_refreshing();
        const lv_coord_t hres = lv_disp_get_hor_res(disp);

        lv_color_t *buf_cpy;
        if (color_p == disp_drv->draw_buf->buf1)
            buf_cpy = static_cast<lv_color_t*>(disp_drv->draw_buf->buf2);
        else
            buf_cpy = static_cast<lv_color_t*>(disp_drv->draw_buf->buf1);

        for (uint16_t area_idx = 0; area_idx < disp->inv_p; area_idx++)
        {
            if (disp->inv_area_joined[area_idx])
                continue;

            const lv_coord_t w = lv_area_get_width(&disp->inv_areas[area_idx]) * sizeof(lv_color_t);
            for (lv_coord_t y = disp->inv_areas[area_idx].y1; y <= disp->inv_areas[area_idx].y2 && y < disp_drv->ver_res; y++)
                memcpy(buf_cpy + (y * hres + disp->inv_areas[area_idx].x1), color_p + (y * hres + disp->inv_areas[area_idx].x1), w);
        }
    }

    lv_disp_flush_ready(disp_drv);
}

}

//-----------------------------------------------------------------------------
/* public */

gui::gui() : active_object("gui", osPriorityNormal, 4096)
{
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, lcd.get_framebuf_1(), lcd.get_framebuf_2(), lcd.width() * lcd.height());
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = lcd.width();
    disp_drv.ver_res = lcd.height();
    disp_drv.flush_cb = gui_disp_flush;
    disp_drv.user_data = &this->lcd;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.direct_mode = 1;
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

void gui::event_handler(const performance_test_evt_t &e)
{
    lv_demo_benchmark();
}


