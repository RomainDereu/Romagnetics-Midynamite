/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */

// List of current select
typedef enum {
    MM_CHANNEL_SELECT = 0,
    MM_VELOCITY_SELECT,
    MM_MIDI_SELECT,

    MT_TRANSPOSE_MODE,

	SETT_START_MENU,
    SETT_SEND_TO_USB,
    SETT_BRIGHTNESS,

	SETT_MIDI_THRU,
    SETT_USB_THRU,
    CHANNEL_FILTER,

	FT1, FT2, FT3, FT4, FT5, FT6, FT7, FT8,
	FT9, FT10, FT11, FT12, FT13, FT14, FT15, FT16,


    ABOUT,
    AMOUNT_OF_SETTINGS
} current_select_list_t;


#define BOTTOM_LINE_VERT LINE_4_VERT + 3

#include "cmsis_os.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"
#include "main.h"
#include "menu.h"
#include "utils.h"
#include "settings.h"
#include "text.h"

extern osThreadId display_updateHandle;

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;
extern midi_transpose_data_struct midi_transpose_data;
extern settings_data_struct settings_data;

extern const Message *message;

// Array with all the possible select values. Is being used to update the UI
static uint8_t select_states[AMOUNT_OF_SETTINGS] = {0};

// Contrast value list
static uint8_t contrast_values[10] = {0x39, 0x53, 0x6D, 0x87, 0xA1, 0xBB, 0xD5, 0xEF, 0xF9, 0xFF};

// Contrast index and current/old selection tracking
static uint8_t contrast_index;
static uint8_t current_select = 0;
static uint8_t old_select = 0;

// The current selected menu part
void screen_update_settings(){
	screen_driver_Fill(Black);
	if(current_select >= MM_CHANNEL_SELECT && current_select <= MM_MIDI_SELECT){
		screen_update_settings_midi_modify();
	}
	else if (current_select == MT_TRANSPOSE_MODE){
		screen_update_settings_midi_transpose();
	}
	else if (current_select >= SETT_START_MENU && current_select <= SETT_BRIGHTNESS){
		screen_update_global_settings1();
	}
	else if (current_select >= SETT_MIDI_THRU && current_select <= CHANNEL_FILTER){
		screen_update_global_settings2();
	}
	else if (current_select >= FT1 && current_select <= FT16){
		screen_update_midi_filter();
	}
	else if (current_select == ABOUT){
		screen_update_settings_about();
	}
	screen_driver_Line(0, LINE_4_VERT, 127, LINE_4_VERT, White);
	screen_driver_SetCursor_WriteString(message->save_instruction, Font_6x8, White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	screen_driver_UpdateScreen();
}

// Midi Modify page
void screen_update_settings_midi_modify(){
	menu_display(&Font_6x8, message->settings_modify);

	// Channel Modify
	screen_driver_SetCursor_WriteString(message->midi_modify_select, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	const char * split_choice = message->choices.change_split[midi_modify_data.change_or_split];
	screen_driver_underline_WriteString(split_choice, Font_6x8, White, 80, LINE_1_VERT, select_states[MM_CHANNEL_SELECT]);

	// Velocity
	screen_driver_SetCursor_WriteString(message->velocity, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	const char * velocity_choice = message->choices.change_fixed[midi_modify_data.velocity_type];
	screen_driver_underline_WriteString(velocity_choice, Font_6x8, White, 80, LINE_2_VERT, select_states[MM_VELOCITY_SELECT]);

	// Channel
	screen_driver_SetCursor_WriteString(message->send_to, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	const char * midi_out_choice = message->choices.midi_outs[midi_modify_data.send_to_midi_out];
	screen_driver_underline_WriteString(midi_out_choice, Font_6x8, White, 60, LINE_3_VERT, select_states[MM_MIDI_SELECT]);


}

// Transpose section
void screen_update_settings_midi_transpose(){
	menu_display(&Font_6x8, message->settings_transpose);

	// Transpose Mode
	screen_driver_SetCursor_WriteString(message->type, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	const char * transpose_type = message->choices.transpose_modes[midi_transpose_data.transpose_type];
	screen_driver_underline_WriteString(transpose_type, Font_6x8, White, 60, LINE_1_VERT, select_states[MT_TRANSPOSE_MODE]);

}

// Settings Section
void screen_update_global_settings1(){
	menu_display(&Font_6x8, message->global_settings_1);

	// Start Menu
	screen_driver_SetCursor_WriteString(message->start_menu, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	const char *start_menu_options[] = {
		message->tempo,
		message->modify,
		message->transpose,
		message->settings,
	};
	screen_driver_underline_WriteString(start_menu_options[settings_data.start_menu], Font_6x8, White, 70, LINE_1_VERT, select_states[SETT_START_MENU]);

	// Send to USB
	screen_driver_SetCursor_WriteString(message->usb_midi, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_underline_WriteString(message->choices.usb_receive_send[settings_data.send_to_usb], Font_6x8, White, 70, LINE_2_VERT, select_states[SETT_SEND_TO_USB]);

	// Contrast
	screen_driver_SetCursor_WriteString(message->contrast, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	screen_driver_underline_WriteString(message->contrast_levels[contrast_index], Font_6x8, White, 70, LINE_3_VERT, select_states[SETT_BRIGHTNESS]);
}


void screen_update_global_settings2(){
	menu_display(&Font_6x8, message->global_settings_2);

	// MIDI THRU
	screen_driver_SetCursor_WriteString(message->MIDI_Thru, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[settings_data.midi_thru], Font_6x8, White, 80, LINE_1_VERT, select_states[SETT_MIDI_THRU]);

	// USB THRU
	screen_driver_SetCursor_WriteString(message->USB_Thru, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[settings_data.usb_thru], Font_6x8, White, 80, LINE_2_VERT, select_states[SETT_USB_THRU]);

	// MIDI
	screen_driver_SetCursor_WriteString(message->MIDI_Filter, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[settings_data.channel_filter], Font_6x8, White, 80, LINE_3_VERT, select_states[CHANNEL_FILTER]);
}

void screen_update_midi_filter(){
	menu_display(&Font_6x8, message->USB_Thru);
	screen_driver_SetCursor_WriteString(message->X_equals_ignore_channel, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);


	for (uint8_t i = 0; i < 16; i++) {
	    char label[3];  // Enough for "16" or "X"

	    // Show "X" if bit is cleared, else show channel number
	    if ((settings_data.filtered_channels & (1U << i)) != 0) {
	        strcpy(label, "X");  // Bit = 1 → channel is blocked
	    } else {
	        snprintf(label, sizeof(label), "%u", i + 1);  // Bit = 0 → channel allowed
	    }

	    uint8_t x = 5 + 15 * (i % 8);
	    uint8_t y = (i < 8) ? LINE_2_VERT : LINE_3_VERT;

	    screen_driver_underline_WriteString(label, Font_6x8, White, x, y, select_states[FT1 + i]);
	}


}

// About Section
void screen_update_settings_about(){
	menu_display(&Font_6x8, message->about);
	screen_driver_SetCursor_WriteString(message->about_brand, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	screen_driver_SetCursor_WriteString(message->about_product, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_SetCursor_WriteString(message->about_version, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
}

// Settings update logic
void settings_update_menu(TIM_HandleTypeDef * timer3,
                          TIM_HandleTypeDef * timer4,
                          uint8_t * old_menu){

	contrast_index = calculate_contrast_index(settings_data.brightness);

	midi_modify_data_struct old_modify_data = midi_modify_data;
	midi_transpose_data_struct old_midi_transpose_data = midi_transpose_data;
	settings_data_struct old_settings_data = settings_data;

	uint8_t menu_changed = (*old_menu != SETTINGS);
	utils_counter_change(timer3, &current_select, 0, AMOUNT_OF_SETTINGS-1, menu_changed, 1, WRAP);

	// Compute whether the selection changed before the switch
	uint8_t select_changed = (old_select != current_select);
	switch (current_select) {
		// Midi Modify section
		case MM_CHANNEL_SELECT:
			utils_counter_change(timer4, &midi_modify_data.change_or_split, 0, 1, select_changed, 1, WRAP);
			break;

		case MM_VELOCITY_SELECT:
			utils_counter_change(timer4, &midi_modify_data.velocity_type, 0, 1, select_changed, 1, WRAP);
			break;

		case MM_MIDI_SELECT:
			utils_counter_change(timer4, &midi_modify_data.send_to_midi_out, MIDI_OUT_1, MIDI_OUT_SPLIT, select_changed, 1, WRAP);
			break;

		// Transpose section
		case MT_TRANSPOSE_MODE:
			utils_counter_change(timer4, &midi_transpose_data.transpose_type, 0, 1, select_changed, 1, WRAP);
			break;

		// Global section
		case SETT_START_MENU:
			utils_counter_change(timer4, &settings_data.start_menu, 0, AMOUNT_OF_MENUS-1, select_changed, 1, WRAP);
			break;

		case SETT_SEND_TO_USB:
			utils_counter_change(timer4, &settings_data.send_to_usb, USB_MIDI_OFF, USB_MIDI_SEND, select_changed, 1, WRAP);
			break;

		case SETT_BRIGHTNESS:
			utils_counter_change(timer4, &contrast_index, 0, 9, select_changed, 1, NO_WRAP);
			if (contrast_index < 10) {
				settings_data.brightness = contrast_values[contrast_index];
				if (old_settings_data.brightness != settings_data.brightness) {
					screen_driver_SetContrast(settings_data.brightness);
				}
			}
			break;
		case SETT_MIDI_THRU:
			utils_counter_change(timer4, &settings_data.midi_thru, 0, 1, select_changed, 1, WRAP);
			break;
		case SETT_USB_THRU:
			utils_counter_change(timer4, &settings_data.usb_thru, 0, 1, select_changed, 1, WRAP);
			break;
		case CHANNEL_FILTER:
			utils_counter_change(timer4, &settings_data.channel_filter, 0, 1, select_changed, 1, WRAP);
			break;

		case FT1: case FT2: case FT3: case FT4:
		case FT5: case FT6: case FT7: case FT8:
		case FT9: case FT10: case FT11: case FT12:
		case FT13: case FT14: case FT15: case FT16: {
		    uint8_t channel_index = current_select - FT1;

		    // Current value of that channel's filter bit (1 = blocked)
		    uint8_t bit_value = (settings_data.filtered_channels >> channel_index) & 1;

		    // Change the bit value (toggle via encoder)
		    utils_counter_change(timer4, &bit_value, 0, 1, select_changed, 1, WRAP);

		    if (bit_value)
		        settings_data.filtered_channels |= (1U << channel_index);  // Set bit → block channel
		    else
		        settings_data.filtered_channels &= ~(1U << channel_index); // Clear bit → allow channel

		    break;
		}
	}


	// Selecting the current item being selected
	for (uint8_t x=0; x < AMOUNT_OF_SETTINGS; x++){
		select_states[x] = 0;
	}
	select_states[current_select] = 1;

	saving_settings_ui();

	if(menu_changed || current_select != old_select ||
		old_modify_data.change_or_split != midi_modify_data.change_or_split ||
		old_modify_data.velocity_type != midi_modify_data.velocity_type ||
		old_modify_data.send_to_midi_out != midi_modify_data.send_to_midi_out ||
		old_midi_transpose_data.transpose_type != midi_transpose_data.transpose_type ||
		old_midi_transpose_data.transpose_scale != midi_transpose_data.transpose_scale ||
		old_midi_transpose_data.send_original != midi_transpose_data.send_original ||
		old_settings_data.start_menu != settings_data.start_menu ||
		old_settings_data.send_to_usb != settings_data.send_to_usb ||

		old_settings_data.usb_thru != settings_data.usb_thru ||
		old_settings_data.midi_thru != settings_data.midi_thru ||
		old_settings_data.filtered_channels != settings_data.filtered_channels ||
		old_settings_data.channel_filter != settings_data.channel_filter ||

		old_settings_data.brightness != settings_data.brightness){
		osThreadFlagsSet(display_updateHandle, 0x08);
	}

	*old_menu = SETTINGS;
	old_select = current_select;
}

// Save portion
void saving_settings_ui(){
	uint8_t Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
	if(Btn1State == 0){
		osDelay(10);
		Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
		if(Btn1State == 0){
			// Saving the current configuration to the memory
			screen_driver_SetCursor_WriteString(message->saving, Font_6x8, White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
			screen_driver_UpdateScreen();

			save_struct memory_to_be_saved = creating_save(&midi_tempo_data,
			                                               &midi_modify_data,
			                                               &midi_transpose_data,
			                                               &settings_data);
			store_settings(&memory_to_be_saved);

			screen_driver_SetCursor_WriteString(message->saved, Font_6x8, White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
			screen_driver_UpdateScreen();
			osDelay(1000);
			screen_driver_SetCursor_WriteString(message->save_instruction, Font_6x8, White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
			screen_driver_UpdateScreen();
		}
	}
}

// Finds index from brightness value
uint8_t calculate_contrast_index(uint8_t brightness) {
	for (uint8_t i = 0; i < sizeof(contrast_values); i++) {
		if (contrast_values[i] == brightness) {
			return i;
		}
	}
	// Default to full brightness if not found
	return 9;
}
