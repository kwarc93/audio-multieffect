/*
 * gui.cpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#include "gui.hpp"

#include "libs/lvgl/lvgl.h"

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

}

//-----------------------------------------------------------------------------
/* public */

gui::gui() : active_object("gui", osPriorityNormal, 4096), lcd { (void*)RGB565_480x272 }
{
    lv_init();

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

void gui::event_handler(const button_evt_t &e)
{

}


