/*
 * midi_modify_transpose_display.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Astaa
 */


#include <stdio.h>


#include "main.h"
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"











void screen_update_midi_transpose(midi_transpose_data_struct * midi_transpose_data){

	screen_driver_Fill(Black);
	menu_display(&Font_6x8, message->midi_transpose);


    screen_driver_UpdateScreen();

}
