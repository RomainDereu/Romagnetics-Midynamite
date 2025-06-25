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

void screen_update_settings();

//Making an array with the list of uarts that will be used
void list_of_UART_to_send_to(uint8_t send_channels,
	                       	 UART_HandleTypeDef **UART_list);

//Declaring the save_struct
save_struct creating_save(midi_tempo_data_struct * midi_tempo_data_to_save,
						  midi_modify_data_struct * midi_modify_data_to_save);

void saving_settings_ui();

HAL_StatusTypeDef  store_settings(save_struct *data);

save_struct read_setting_memory(void);

save_struct make_default_settings(void);

void load_settings();



#endif /* INC_SETTINGS_H_ */
