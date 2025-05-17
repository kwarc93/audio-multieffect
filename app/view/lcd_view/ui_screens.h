/*
 * ui_screens.h
 *
 *  Created on: 17 maj 2025
 *      Author: kwarc
 */

#ifndef UI_SCREENS_H_
#define UI_SCREENS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    lv_obj_t * bypass_button; // lv_button
    lv_obj_t * mix_knob;      // ui_comp_fx_knob
    lv_obj_t * depth_knob; 		 // ui_comp_fx_knob
    lv_obj_t * rate_knob; 		  // ui_comp_fx_knob
    lv_obj_t * mode_switch; 	 // ui_comp_fx_switch
} ui_scr_chorus_t;

lv_obj_t * ui_scr_chorus_create(void);
ui_scr_chorus_t * ui_scr_chorus_get_components(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* UI_SCREENS_H_ */
