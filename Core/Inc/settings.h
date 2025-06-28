/*
 * settings.h
 *
 *  Created on: Jun 25, 2025
 *      Author: Astaa
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "main.h"

void screen_update_settings();
//Different pages of the settings
void screen_update_settings_midi_modify();
void screen_update_settings_midi_transpose();
void screen_update_settings_about();


void settings_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
							 uint8_t * old_menu);

void saving_settings_ui();

#endif /* SETTINGS_H_ */
