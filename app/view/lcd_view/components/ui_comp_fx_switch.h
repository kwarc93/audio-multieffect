/*
 * ui_comp_fx_switch.h
 *
 *  Created on: 16 maj 2025
 *      Author: kwarc
 */

#ifndef UI_COMP_FX_SWITCH_H_
#define UI_COMP_FX_SWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

lv_obj_t * ui_comp_fx_switch_create(lv_obj_t * parent, const char * left_txt, const char * right_txt);

bool ui_comp_fx_switch_get_state(lv_obj_t * widget);
void ui_comp_fx_switch_set_state(lv_obj_t * widget, bool state);
void ui_comp_fx_switch_add_event_cb(lv_obj_t * widget, lv_event_cb_t cb, lv_event_code_t filter, void * user_data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* UI_COMP_FX_SWITCH_H_ */
