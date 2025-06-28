/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Astaa
 */

//List of current select
#define MM_CHANNEL_SELECT 0
#define MM_VELOCITY_SELECT 1
#define MT_TRANSPOSE_MODE 2
#define MT_MIDI_SEND 3
#define ABOUT 4
#define AMOUNT_OF_SETTINGS 5

#include "cmsis_os.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"
#include "menu.h"
#include "utils.h"
#include "settings.h"

extern osThreadId display_updateHandle;

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;
extern midi_transpose_data_struct midi_transpose_data;
extern settings_data_struct settings_data;

char settings_modify_message[30] = "Settings Midi Modify         ";
char settings_transpose_message[30] = "Settings Transpose           ";
char settings_about_message[30] = "About                        ";

//Array with all the possible select values. Is being used to update the UI
uint8_t select_states[AMOUNT_OF_SETTINGS] = {0, 0, 0, 0, 0};

//Lines
uint8_t line_1_vert = 15;
uint8_t line_2_vert = 25;
uint8_t line_3_vert = 35;

//Midi Modify page
char midi_modify_select_message[11] = "Ch. Modify";
char midi_change_message[7] = "Change";
char midi_split_message[6] = "Split";
char * midi_split_choices[2] = {midi_change_message, midi_split_message};


//Velocity
char velocity_select_message[9] = "Velocity";
char velocity_change_message[7] = "Change";
char velocity_fixed_message[6] = "Fixed";
char * velocity_choices[2] = {velocity_change_message, velocity_fixed_message};

//Roro add Midi Transpose section



//About Section
char about_pedal_brand_message[12] = "Romagnetics";
char about_pedal_pedal_message[11] = "Midynamite";
char current_version_message[12] = "Version 1.0";

//Save portion
char save_settings_message[30] = "Press Select to save settings";
char saving_print[30] = "Saving                       ";
char saved_print[30] = "Saved!                       ";

uint8_t current_select = 0;
uint8_t old_select = 0;




//The current selected menu part

void screen_update_settings(){
	screen_driver_Fill(Black);
	if(current_select >= MM_CHANNEL_SELECT && current_select <= MM_VELOCITY_SELECT){
		screen_update_settings_midi_modify();
	}
	else if (current_select >= MT_TRANSPOSE_MODE && current_select <= MT_MIDI_SEND){
		screen_update_settings_midi_transpose();
	}
	else if (current_select == ABOUT){
		screen_update_settings_about();
	}


	//Saving
	screen_driver_SetCursor_WriteString(save_settings_message, Font_6x8, White, 0, 56);


	screen_driver_UpdateScreen();
}

void screen_update_settings_midi_modify(){
	menu_display(&Font_6x8, &settings_modify_message);
	//Midi Mode
	screen_driver_SetCursor_WriteString(midi_modify_select_message, Font_6x8, White, 0, line_1_vert);
	screen_driver_underline_WriteString(midi_split_choices[midi_modify_data.change_or_split],
										Font_6x8, White, 80, line_1_vert, select_states[0]);


	//Velocity
	screen_driver_SetCursor_WriteString(velocity_select_message, Font_6x8, White, 0, line_2_vert);
	screen_driver_underline_WriteString(velocity_choices[midi_modify_data.velocity_type], Font_6x8, White, 80, line_2_vert, select_states[1]);


}

void screen_update_settings_midi_transpose(){

	menu_display(&Font_6x8, &settings_transpose_message);
}

void screen_update_settings_about(){
	screen_driver_SetCursor_WriteString(about_pedal_brand_message, Font_6x8, White, 0, line_1_vert);
	screen_driver_SetCursor_WriteString(about_pedal_pedal_message, Font_6x8, White, 0, line_2_vert);
	screen_driver_SetCursor_WriteString(current_version_message, Font_6x8, White, 0, line_3_vert);
	menu_display(&Font_6x8, &settings_about_message);
}


void settings_update_menu(TIM_HandleTypeDef * timer3,
		                  TIM_HandleTypeDef * timer4,
						  uint8_t * old_menu){


	midi_modify_data_struct old_modify_data = midi_modify_data;

	uint8_t menu_changed = (*old_menu != SETTINGS);
	utils_counter_change(timer3, &current_select, 0, AMOUNT_OF_SETTINGS-1, menu_changed);

	if (current_select == MM_CHANNEL_SELECT ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &midi_modify_data.change_or_split, 0, 1, select_changed);
	}
	else if (current_select == MM_VELOCITY_SELECT ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &midi_modify_data.velocity_type, 0, 1, select_changed);
	}


	//Selecting the current item being selected
	//First resetting everything
	for (uint8_t x=0; x < AMOUNT_OF_SETTINGS; x++){
		select_states[x] = 0;
	}
	select_states[current_select] = 1;

	saving_settings_ui();

	if(menu_changed || current_select!= old_select ||
			old_modify_data.change_or_split != midi_modify_data.change_or_split ||
			old_modify_data.velocity_type != midi_modify_data.velocity_type){
		osThreadFlagsSet(display_updateHandle, 0x08);
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

				 save_struct memory_to_be_saved = creating_save(&midi_tempo_data,
						 	 	 	 	 	 	 	 	 	 	&midi_modify_data,
																&midi_transpose_data,
																&settings_data);
				 store_settings(&memory_to_be_saved);

				 screen_driver_SetCursor_WriteString(saved_print, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();
				 osDelay(1000);
				 screen_driver_SetCursor_WriteString(save_settings_message, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();
				  }
	  }

}
