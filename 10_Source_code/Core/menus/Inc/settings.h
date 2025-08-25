/*
 * settings.h
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "cmsis_os.h"
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
    AMOUNT_OF_SETTINGS_ITEMS
} settings_select_list_t;

void screen_update_settings();
void settings_update_menu();

#endif /* SETTINGS_H_ */
