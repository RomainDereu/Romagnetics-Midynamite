/*
 * settings.h
 *
 *  Created on: Apr 2, 2025
 *      Author: Astaa
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include "stdio.h"
#include "main.h"

//Declaring the save_struct
struct save_struct creating_save(struct midi_tempo_data_struct * midi_tempo_data_to_save);


void screen_update_settings();

void settings_saved();

HAL_StatusTypeDef  store_settings(struct save_struct *data);
struct save_struct  read_settings(void);



#endif /* INC_SETTINGS_H_ */
