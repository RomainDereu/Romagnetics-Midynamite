/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Astaa
 */

#include "cmsis_os2.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"
#include "menu.h"
#include "utils.h"
#include "settings.h"

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;

char message_settings[30] = "Settings                      ";
char velocity_select_message[15] = "Velocity Select";

char velocity_change_message[6] = "Change";
char velocity_fixed_message[6] = "Fixed ";
char * velocity_choices[2] = {&velocity_change_message, &velocity_fixed_message};

char save_settings_message[29] = "Press Select to save settings";
char saving_print[29] = "Saving                       ";
char saved_print[29] = "Saved!                       ";

uint8_t current_select = 0;
uint8_t old_select = 0;

uint8_t current_value = 0;
uint8_t old_value = 0;

uint8_t select_states[2] = {0, 0};


//The current selected menu part

void screen_update_settings(){
	screen_driver_Fill(Black);
	menu_display(&Font_6x8, &message_settings);
	screen_driver_underline_WriteString(velocity_select_message, Font_6x8, White, 0, 15, select_states[0]);
	screen_driver_underline_WriteString(save_settings_message, Font_6x8, White, 0, 50, select_states[1]);

	screen_driver_UpdateScreen();
}


void settings_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
							 uint8_t * old_menu){
	uint8_t menu_changed = (*old_menu != SETTINGS);
	utils_counter_change(timer3, &current_select, 0, 1, menu_changed);
	//Selecting the current_menu
	if (current_select == 0 ){
		select_states[0] = 1;
		select_states[1] = 0;
	}
	else if (current_select == 1 ){
		select_states[0] = 0;
		select_states[1] = 1;
		}

	saving_settings_ui();

	if(menu_changed || current_select!= old_select || current_value!= old_value){
		screen_update_settings();
	}

	*old_menu = SETTINGS;
	old_select = current_select;
	old_value = current_value;


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
				 screen_driver_SetCursor_WriteString(saving_print, Font_6x8, White, 0, 50);
				 screen_driver_UpdateScreen();

				 save_struct memory_to_be_saved = creating_save(&midi_tempo_data, &midi_modify_data);
				 store_settings(&memory_to_be_saved);

				 screen_driver_SetCursor_WriteString(saved_print, Font_6x8, White, 0, 50);
				 screen_driver_UpdateScreen();
				 osDelay(1000);
				 screen_driver_SetCursor_WriteString(save_settings_message, Font_6x8, White, 0, 50);
				 screen_driver_UpdateScreen();
				  }
	  }

}
