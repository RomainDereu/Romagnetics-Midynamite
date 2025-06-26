/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Astaa
 */

#define CHANNEL_SELECT 0
#define VELOCITY_SELECT 1

#include "cmsis_os2.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"
#include "menu.h"
#include "utils.h"
#include "settings.h"

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;
extern settings_data_struct settings_data;

char message_settings[30] = "Settings Midi Modify        ";

//Midi mode mode
char midi_modify_select_message[11] = "Ch. Modify";
char midi_change_message[7] = "Change";
char midi_split_message[6] = "Split";
char * midi_split_choices[2] = {midi_change_message, midi_split_message};
uint8_t midi_vert_pos = 15;

//Velocity
char velocity_select_message[9] = "Velocity";
char velocity_change_message[7] = "Change";
char velocity_fixed_message[6] = "Fixed";
char * velocity_choices[2] = {velocity_change_message, velocity_fixed_message};
uint8_t velocity_vert_pos = 25;



char save_settings_message[30] = "Press Select to save settings";
char saving_print[30] = "Saving                       ";
char saved_print[30] = "Saved!                       ";

uint8_t current_select = 0;
uint8_t old_select = 0;

uint8_t select_states[2] = {0, 0};


//The current selected menu part

void screen_update_settings(){
	screen_driver_Fill(Black);
	menu_display(&Font_6x8, &message_settings);
	//Midi Mode
	screen_driver_SetCursor_WriteString(midi_modify_select_message, Font_6x8, White, 0, midi_vert_pos);
	screen_driver_underline_WriteString(midi_split_choices[settings_data.midi_channel_mode], Font_6x8, White, 80, midi_vert_pos, select_states[0]);


	//Velocity
	screen_driver_SetCursor_WriteString(velocity_select_message, Font_6x8, White, 0, velocity_vert_pos);
	screen_driver_underline_WriteString(velocity_choices[settings_data.midi_velocity_mode], Font_6x8, White, 80, velocity_vert_pos, select_states[1]);

	//Saving
	screen_driver_SetCursor_WriteString(save_settings_message, Font_6x8, White, 0, 56);

	screen_driver_UpdateScreen();
}


void settings_update_menu(TIM_HandleTypeDef * timer3,
		                  TIM_HandleTypeDef * timer4,
						  uint8_t * old_menu){

	//Saving the settings struct to check if the screen must be updated later
	settings_data_struct old_settings_data = settings_data;

	uint8_t menu_changed = (*old_menu != SETTINGS);
	utils_counter_change(timer3, &current_select, 0, 1, menu_changed);

	if (current_select == CHANNEL_SELECT ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &settings_data.midi_channel_mode, 0, 1, select_changed);
	}
	else if (current_select == VELOCITY_SELECT ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &settings_data.midi_velocity_mode, 0, 1, select_changed);
	}


	//Selecting the current_menu
	if (current_select == 0 ){
		select_states[CHANNEL_SELECT] = 1;
		select_states[VELOCITY_SELECT] = 0;
	}
	else if (current_select == 1 ){
		select_states[CHANNEL_SELECT] = 0;
		select_states[VELOCITY_SELECT] = 1;
		}

	saving_settings_ui();

	if(menu_changed || current_select!= old_select ||
			old_settings_data.midi_channel_mode != settings_data.midi_channel_mode ||
			old_settings_data.midi_velocity_mode != settings_data.midi_velocity_mode){
		screen_update_settings();
	}

	*old_menu = SETTINGS;
	old_select = current_select;

}


void saving_settings_ui(){
	uint8_t Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
	  if(Btn1State == 0)
		 {
			 //Debouncing
			 osDelay(10);
			 Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
			  if(Btn1State == 0)
				 {
				 //Saving the current configuration to the memory
				 screen_driver_SetCursor_WriteString(saving_print, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();

				 save_struct memory_to_be_saved = creating_save(&midi_tempo_data, &midi_modify_data, &settings_data);
				 store_settings(&memory_to_be_saved);

				 screen_driver_SetCursor_WriteString(saved_print, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();
				 osDelay(1000);
				 screen_driver_SetCursor_WriteString(save_settings_message, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();
				  }
	  }

}
