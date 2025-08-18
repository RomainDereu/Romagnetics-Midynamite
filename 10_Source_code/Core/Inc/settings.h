/*
 * settings.h
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "main.h"

// List of current select
typedef enum {


	SETT_START_MENU = 0,
    SETT_SEND_TO_USB,
    SETT_BRIGHTNESS,

	SETT_MIDI_THRU,
    SETT_USB_THRU,
    CHANNEL_FILTER,

	FT1, FT2, FT3, FT4, FT5, FT6, FT7, FT8,
	FT9, FT10, FT11, FT12, FT13, FT14, FT15, FT16,


    ABOUT,
    AMOUNT_OF_SETTINGS
} current_select_list_t;

extern uint8_t select_states[];
extern uint8_t contrast_values[];

extern uint8_t contrast_index;
extern uint8_t current_select;
extern uint8_t old_select;

void screen_update_settings();

//Different pages of the settings
void screen_update_global_settings1();
void screen_update_global_settings2();
void screen_update_midi_filter();
void screen_update_settings_about();


void settings_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
							 uint8_t * old_menu);

#endif /* SETTINGS_H_ */
