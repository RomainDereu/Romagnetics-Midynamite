/*
 * settings.h
 *
 *  Created on: Jun 25, 2025
 *      Author: Astaa
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "main.h"

void settings_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu);

void saving_settings_ui();

#endif /* SETTINGS_H_ */
