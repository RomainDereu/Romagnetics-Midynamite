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
save_struct creating_save(midi_tempo_data_struct * midi_tempo_data_to_save,
		                  midi_modify_data_struct * midi_modify_data_to_save,
						  settings_data_struct *settings_data_to_save);



HAL_StatusTypeDef  store_settings(save_struct *data);

save_struct read_setting_memory(void);

save_struct make_default_settings(void);

void load_settings();



#endif /* INC_SETTINGS_H_ */
