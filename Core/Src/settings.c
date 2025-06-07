/*
 * settings.c
 *
 *  Created on: Apr 2, 2025
 *      Author: Astaa
 */


#include "cmsis_os.h"
#include "settings.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"


void screen_update_settings(){
char save_settings_message[30] = "Save Settings                 ";
screen_driver_SetCursor(0, 56);
screen_driver_WriteString(save_settings_message, Font_6x8, White);
screen_driver_UpdateScreen();
}


void settings_saved(){
	uint8_t Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
	  if(Btn1State == 0)
		 {
			 //Debouncing
			 osDelay(10);
			 Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
			  if(Btn1State == 0)
				 {
				 //Romain add the save functions here
				 char saved_print[6] = "Saved!";
				 char saved__clear_print[6] = "      ";
				 screen_driver_SetCursor(80, 56);
				 screen_driver_WriteString(saved_print, Font_6x8, White);
				 screen_driver_UpdateScreen();
				 //Delay to allow for continuous pressing of the button
				 osDelay(1000);
				 screen_driver_SetCursor(80, 56);
				 screen_driver_WriteString(saved__clear_print, Font_6x8, White);
				 screen_driver_UpdateScreen();
				  }
	  }




}
