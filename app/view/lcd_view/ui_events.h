// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: gmfx

#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

void ui_settings_in_vol_changed(lv_event_t * e);
void ui_settings_out_vol_changed(lv_event_t * e);
void ui_cab_sim_bypass(lv_event_t * e);
void ui_tremolo_bypass(lv_event_t * e);
void ui_tremolo_rate_changed(lv_event_t * e);
void ui_tremolo_depth_changed(lv_event_t * e);
void ui_tremolo_shape_changed(lv_event_t * e);
void ui_echo_bypass(lv_event_t * e);
void ui_echo_blend_changed(lv_event_t * e);
void ui_echo_feedb_changed(lv_event_t * e);
void ui_echo_time_changed(lv_event_t * e);
void ui_echo_mode_changed(lv_event_t * e);
void ui_overdrive_bypass(lv_event_t * e);
void ui_overdrive_low_changed(lv_event_t * e);
void ui_overdrive_gain_changed(lv_event_t * e);
void ui_overdrive_high_changed(lv_event_t * e);
void ui_overdrive_mode_changed(lv_event_t * e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif