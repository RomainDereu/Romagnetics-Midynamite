/*
 * midi_modify_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Astaa
 */


#include <stdio.h>
#include <stdint.h>

#include "midi_modify.h"
#include "cmsis_os.h"

#include "menu.h"
#include "utils.h"
#include "main.h"


static uint8_t select_states[5] = {0};
static uint8_t current_select = 0;
static uint8_t old_select = 0;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern osThreadId display_updateHandle;


void midi_transpose_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_transpose_data_struct * midi_transpose_data,
							 uint8_t * old_menu){

	uint8_t menu_changed = (*old_menu != MIDI_TRANSPOSE);

	if (menu_changed == 1){
		osThreadFlagsSet(display_updateHandle, 0x04);
	}

	old_select = current_select;

}
