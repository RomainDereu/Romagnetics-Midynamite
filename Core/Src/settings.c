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
char save_settings_message[13] = "Save Settings";

char saved__clear_print[6] = "      ";
char saving_print[6] = "Saving";
char saved_print[6] = "Saved!";

//The current selected menu part


void settings_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu){
	uint8_t menu_changed = (*old_menu != SETTINGS);
	if (menu_changed) {
			screen_driver_Fill(Black);
		}



	saving_settings_ui();
		if(* old_menu != SETTINGS){
		screen_driver_Fill(Black);
		screen_driver_underline_WriteString(save_settings_message, Font_6x8, White, 0, 54, 1);

		}

	menu_display(&Font_6x8, &message_settings);
	screen_driver_UpdateScreen();
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


				 screen_driver_SetCursor(90, 56);
				 screen_driver_WriteString(saving_print, Font_6x8, White);


				 screen_driver_UpdateScreen();

				 save_struct memory_to_be_saved = creating_save(&midi_tempo_data, &midi_modify_data);
				 store_settings(&memory_to_be_saved);


				 screen_driver_SetCursor(90, 56);
				 screen_driver_WriteString(saved_print, Font_6x8, White);
				 screen_driver_UpdateScreen();
				 osDelay(1000);
				 screen_driver_SetCursor(90, 56);
				 screen_driver_WriteString(saved__clear_print, Font_6x8, White);
				 screen_driver_UpdateScreen();
				  }
	  }

}
