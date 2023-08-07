/*
 * gui.cpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#include "gui.hpp"

#include "libs/lvgl/lvgl.h"
#include "libs/lvgl/demos/lv_demos.h"

#include "middlewares/i2c_manager.hpp"

#include <drivers/stm32f7/ltdc.hpp>
#include <drivers/stm32f7/gpio.hpp>
#include <drivers/stm32f7/dma2d.hpp>

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

lv_disp_drv_t lvgl_disp_drv;
lv_indev_drv_t lvgl_indev_drv;
lv_disp_draw_buf_t lvgl_draw_buf;
osSemaphoreId_t vsync_semph;
osSemaphoreId_t dma2d_semph;
drivers::gpio::io vsync_io {drivers::gpio::port::portc, drivers::gpio::pin::pin7};
//hal::displays::main::pixel_t *current_fb;
//volatile bool copying_fb;

void lvgl_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    using display_t = hal::displays::main;
    display_t *display = static_cast<display_t*>(disp_drv->user_data);

    if constexpr (display_t::use_double_framebuf)
    {
        static display_t::pixel_t *front_fb = display->get_frame_buffers().first.data();

        display_t::pixel_t *back_fb;
        if (front_fb == display->get_frame_buffers().first.data())
            back_fb = display->get_frame_buffers().second.data();
        else
            back_fb = display->get_frame_buffers().first.data();

        drivers::dma2d::transfer_cfg cfg
        {
            [](){ osSemaphoreRelease(dma2d_semph);},
                    drivers::dma2d::mode::mem_to_mem,
                    drivers::dma2d::color::RGB565,
            255,
            color_p,
            back_fb,
            480, 272,
            area->x1, area->y1, area->x2, area->y2,
            false
        };

        drivers::dma2d::transfer(cfg);
        osSemaphoreAcquire(dma2d_semph, osWaitForever);

//        display->draw_data(area->x1, area->y1, area->x2, area->y2, reinterpret_cast<display_t::pixel_t*>(color_p));

        if (lv_disp_flush_is_last(disp_drv))
        {
//            display->wait_for_vsync();
            if (osSemaphoreGetCount(vsync_semph))
            {
                /* this means that lvgl rendered frame slower than refresh rate of LCD */
                /* we must skip this and wait for next vsync */
                osSemaphoreAcquire(vsync_semph, 0);
            }
            osSemaphoreAcquire(vsync_semph, osWaitForever);

            /* Set front buffer */
            display->set_frame_buffer(back_fb);

            /* Copy front buffer to back buffer */
            cfg.src = back_fb;
            cfg.dst = front_fb;
            cfg.x1 = 0; cfg.y1 = 0; cfg.x2 = 480-1; cfg.y2 = 272-1;
            drivers::dma2d::transfer(cfg);
            osSemaphoreAcquire(dma2d_semph, osWaitForever);

            front_fb = back_fb;
        }

        lv_disp_flush_ready(&lvgl_disp_drv);
    }
    else
    {
        display->draw_data(area->x1, area->y1, area->x2, area->y2, reinterpret_cast<display_t::pixel_t*>(color_p));
    }
}

void lvgl_disp_render_start(struct _lv_disp_drv_t * disp_drv)
{
    using display_t = hal::displays::main;
    display_t *display = static_cast<display_t*>(disp_drv->user_data);
//    display->wait_for_vsync();
}

void lvgl_input_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    using display_t = hal::displays::main;
    display_t *display = static_cast<display_t*>(drv->user_data);

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

void lvgl_wait(struct _lv_disp_drv_t * disp_drv)
{
//    osSemaphoreAcquire(lvgl_semph, osWaitForever);
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

gui::gui() : active_object("gui", osPriorityNormal, 4096),
display {middlewares::i2c_managers::main::get_instance()}
{
    lv_init();

//    if constexpr (display.use_double_framebuf)
//    {
//        hal::displays::main::pixel_t *fb1 = display.get_frame_buffers().first.data();
//        hal::displays::main::pixel_t *fb2 = display.get_frame_buffers().second.data();
//        lv_disp_draw_buf_init(&lvgl_draw_buf, fb1, fb2, display.width() * display.height());
//    }
//    else
//    {
        __attribute__((section(".dtcmram"))) static lv_color_t lvgl_buf[64 * 1024 / sizeof(lv_color_t)];
        lv_disp_draw_buf_init(&lvgl_draw_buf, lvgl_buf, NULL, sizeof(lvgl_buf) / sizeof(lv_color_t));
//    }

    lv_disp_drv_init(&lvgl_disp_drv);
    lvgl_disp_drv.hor_res = display.width();
    lvgl_disp_drv.ver_res = display.height();
    lvgl_disp_drv.flush_cb = lvgl_disp_flush;
    lvgl_disp_drv.render_start_cb = lvgl_disp_render_start;
    lvgl_disp_drv.wait_cb = lvgl_wait;
    lvgl_disp_drv.user_data = &this->display;
    lvgl_disp_drv.draw_buf = &lvgl_draw_buf;
//    lvgl_disp_drv.full_refresh = display.use_double_framebuf;
    lv_disp_drv_register(&lvgl_disp_drv);

    lv_indev_drv_init(&lvgl_indev_drv);
    lvgl_indev_drv.type = LV_INDEV_TYPE_POINTER;
    lvgl_indev_drv.read_cb = lvgl_input_read;
    lvgl_indev_drv.user_data = &this->display;
    lv_indev_drv_register(&lvgl_indev_drv);


    drivers::gpio::configure(vsync_io);

    vsync_semph = osSemaphoreNew(1, 0, nullptr);
    assert(vsync_semph != nullptr);
    dma2d_semph = osSemaphoreNew(1, 0, nullptr);
    assert(dma2d_semph != nullptr);

    display.vsync(display.use_double_framebuf);
    display.set_vsync_callback([](){ osSemaphoreRelease(vsync_semph); });
    display.set_draw_callback([](){ osSemaphoreRelease(dma2d_semph); });
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
    lv_demo_widgets();
}


