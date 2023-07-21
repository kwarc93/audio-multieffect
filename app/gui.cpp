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

#include "RGB565_480x272.h" // Test image, to be removed

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

void gui_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    hal::lcd_tft_480x272 *lcd = static_cast<hal::lcd_tft_480x272*>(disp_drv->user_data);
    lcd->set_framebuff(color_p);
    lv_disp_flush_ready(disp_drv);
}

}

//-----------------------------------------------------------------------------
/* public */

gui::gui() : active_object("gui", osPriorityNormal, 4096), lcd { (void*)RGB565_480x272 }
{
    lv_init();

    __attribute__((section(".sdram"))) static lv_color_t buf_1_1[480 * 272];
    __attribute__((section(".sdram"))) static lv_color_t buf_1_2[480 * 272];

    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf_1_1, buf_1_2, 480 * 272);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = 480;
    disp_drv.ver_res = 272;
    disp_drv.flush_cb = gui_disp_flush;
    disp_drv.user_data = &this->lcd;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;
//    disp_drv.direct_mode = 1;
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


