/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "cmsis_os.h"
#include "main.h"
#include "menu.h"
#include "midi_modify.h"
#include "utils.h"
#include "main.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"


#define BOTTOM_LINE_VERT LINE_4_VERT + 3


extern osThreadId display_updateHandle;
extern const Message * message;


static uint8_t  current_select = 0;
static uint8_t  old_select = 0;
static uint8_t select_states[4] = {0};




//midi modify menu
void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu){

	midi_modify_data_struct old_modify_data = * midi_modify_data;
	uint8_t menu_changed = (*old_menu != MIDI_MODIFY);


	//The amount of values to be changed depends on the MIDI_MODIFY setting
    uint8_t amount_of_settings = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 2 : 4;

	//Updating the selected item and see if it has changed
	utils_counter_change(timer3, &current_select, 0, amount_of_settings-1, menu_changed, 1, WRAP);
	uint8_t select_changed = (old_select != current_select);
	// Finding urrent item being selected
	for (uint8_t x=0; x < amount_of_settings; x++){
		select_states[x] = 0;
	}
	select_states[current_select] = 1;

	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
		switch (current_select) {
			case 0:
				utils_counter_change(timer4, &(midi_modify_data->send_to_midi_channel_1), 1, 16, select_changed, 1, NO_WRAP);
				break;
			case 1:
				if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
					utils_counter_change_i32(timer4, &(midi_modify_data->velocity_plus_minus), -50, 50, select_changed, 10, NO_WRAP);
					break;
				}
				else if (midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
					utils_counter_change(timer4, &(midi_modify_data->velocity_absolute), 0, 127, select_changed, 10, NO_WRAP);
					break;
			}
		}

	}


	else if (midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
		switch (current_select) {
		case 0:
			utils_counter_change(timer4, &(midi_modify_data->split_midi_channel_1), 1, 16, select_changed, 1, NO_WRAP);
			break;
		case 1:
			utils_counter_change(timer4, &(midi_modify_data->split_midi_channel_2), 1, 16, select_changed, 1, NO_WRAP);
			break;
		case 2:
			utils_counter_change(timer4, &(midi_modify_data->split_note), 0, 127, select_changed, 12, NO_WRAP);
			break;
		case 3:
			if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
				utils_counter_change_i32(timer4, &(midi_modify_data->velocity_plus_minus), -50, 50, select_changed, 10, NO_WRAP);
				break;
			}
			else if (midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
				utils_counter_change(timer4, &(midi_modify_data->velocity_absolute), 0, 127, select_changed, 10, NO_WRAP);
				break;
			}
		}

	}

	static uint8_t Btn1PrevState = 1;
    uint8_t Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
    if(Btn1State == 0 && Btn1PrevState == 1){
    	osDelay(50);
    	Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
        if(Btn1State == 0){
        	//Toggling the type based on the current select
        	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
        		switch (current_select) {

        			case 0:
        				utils_change_settings(&midi_modify_data->change_or_split, 0, 1);
        				break;

        			case 1:
        				utils_change_settings(&midi_modify_data->velocity_type, 0, 1);
        				break;
        		}
    		}


        	else if (midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
        		switch (current_select) {

					case 0:
					case 1:
					case 2:
						utils_change_settings(&midi_modify_data->change_or_split, 0, 1);
						break;

					case 3:
						utils_change_settings(&midi_modify_data->velocity_type, 0, 1);
						break;
        			}
        		}
        }

    }
    Btn1PrevState = Btn1State;





	if (menu_changed == 1 || old_select != current_select ||
		old_modify_data.send_to_midi_channel_1 != midi_modify_data->send_to_midi_channel_1 ||
		old_modify_data.send_to_midi_channel_2 != midi_modify_data->send_to_midi_channel_2 ||


		old_modify_data.split_note != midi_modify_data->split_note ||

		old_modify_data.split_midi_channel_1 != midi_modify_data->split_midi_channel_1 ||
		old_modify_data.split_midi_channel_2 != midi_modify_data->split_midi_channel_2 ||

		old_modify_data.velocity_plus_minus != midi_modify_data->velocity_plus_minus ||
		old_modify_data.velocity_absolute != midi_modify_data->velocity_absolute ||

		old_modify_data.change_or_split != midi_modify_data->change_or_split ||
		old_modify_data.velocity_type != midi_modify_data->velocity_type)
	 {
		osThreadFlagsSet(display_updateHandle, 0x02);
		}
	*old_menu = MIDI_MODIFY;
	old_select = current_select;
}





void screen_update_midi_modify(midi_modify_data_struct * midi_modify_data){
	screen_driver_Fill(Black);

	menu_display(&Font_6x8, message->midi_modify);

	//Top part of the screen (Channel)
	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
		screen_update_channel_change(midi_modify_data);
	}

	else if(midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
		screen_update_channel_split(midi_modify_data);
	}

	screen_driver_Line(0, LINE_4_VERT, 127, LINE_4_VERT, White);

	//Bottom part of the screen (velocity)
	if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
		screen_update_velocity_change(midi_modify_data);
	}

	else if(midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
		screen_update_velocity_fixed(midi_modify_data);
	}

	//On/Off part
	midi_modify_on_off(midi_modify_data->currently_sending, LINE_4_VERT);

    screen_driver_UpdateScreen();

}


//On/ Off Part
void midi_modify_on_off(uint8_t on_or_off, uint8_t bottom_line){
	screen_driver_Line(92, 10, 92, bottom_line, White);
	uint8_t text_position = bottom_line/2;
    screen_driver_SetCursor(95, text_position);

    if(on_or_off ==0){
  	  screen_driver_WriteString(message->off, Font_11x18 , White);
    }
    else if (on_or_off ==1){
  	  screen_driver_WriteString(message->on, Font_11x18 , White);
    }

}



//Channel
void screen_update_channel_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    uint8_t channel = midi_modify_data->send_to_midi_channel_1;

    screen_driver_SetCursor_WriteString(message->to_channel, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);
    char channel_text[5];
    sprintf(channel_text, "%d", channel);
    screen_driver_underline_WriteString(channel_text, Font_6x8 , White, 70, LINE_2_VERT, select_states[0]);
}


void screen_update_channel_split(midi_modify_data_struct * midi_modify_data){

    screen_driver_SetCursor_WriteString(message->low_to_ch, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    uint8_t low_channel = midi_modify_data->split_midi_channel_1;
    char low_channel_text[6];
    sprintf(low_channel_text, "%d" , low_channel);
    screen_driver_underline_WriteString(low_channel_text, Font_6x8, White, 70, LINE_1_VERT, select_states[0]);

    screen_driver_SetCursor_WriteString(message->high_to_ch, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);
    uint8_t high_channel = midi_modify_data->split_midi_channel_2;
    char high_channel_text[6];
    sprintf(high_channel_text, "%d", high_channel);
    screen_driver_underline_WriteString(high_channel_text, Font_6x8, White, 70, LINE_2_VERT, select_states[1]);

	screen_driver_SetCursor_WriteString(message->split, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
    const char * note_to_write = message->midi_note_names[midi_modify_data->split_note];
    //Needs more clearance than the other items due to sharps and flats
    screen_driver_underline_WriteString(note_to_write, Font_6x8, White, 40, LINE_3_VERT, select_states[2]);


}


//Velocity
void screen_update_velocity_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 1 : 3;
	int8_t plus_minus_i8 = midi_modify_data->velocity_plus_minus;
    char modify_value[5];
    sprintf(modify_value, "%d", plus_minus_i8);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, BOTTOM_LINE_VERT, select_states[current_line]);
}
void screen_update_velocity_fixed(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->fixed_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 1 : 3;
    char modify_value[5];
    sprintf(modify_value, "%d", midi_modify_data->velocity_absolute);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, LINE_4_VERT+3, select_states[current_line]);
}


