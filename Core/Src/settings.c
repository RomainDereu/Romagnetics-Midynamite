/*
 * settings.c
 *
 *  Created on: Apr 2, 2025
 *      Author: Astaa
 */


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




}
