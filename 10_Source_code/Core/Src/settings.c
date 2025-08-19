/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */
#define BOTTOM_LINE_VERT LINE_4_VERT + 3

#include <string.h>

#include "cmsis_os.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"
#include "main.h"
#include "menu.h"
#include "utils.h"
#include "settings.h"
#include "text.h"

extern osThreadId_t display_updateHandle;

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;
extern midi_transpose_data_struct midi_transpose_data;
extern settings_data_struct settings_data;

extern const Message *message;


static uint8_t calculate_contrast_index(uint8_t brightness) {
	uint8_t contrast_values[10] = {0x39, 0x53, 0x6D, 0x87, 0xA1, 0xBB, 0xD5, 0xEF, 0xF9, 0xFF};
	for (uint8_t i = 0; i < sizeof(contrast_values); i++) {
		if (contrast_values[i] == brightness) {
			return i;
		}
	}
	// Default to full brightness if not found
	return 9;
}


// Settings Section
static void screen_update_global_settings1(uint8_t *select_states){
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
	uint8_t idx = calculate_contrast_index(settings_data.brightness);
	screen_driver_underline_WriteString(message->contrast_levels[idx], Font_6x8, White, 70, LINE_3_VERT, select_states[SETT_BRIGHTNESS]);
}


static void screen_update_global_settings2(uint8_t *select_states){
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

static void screen_update_midi_filter(uint8_t *select_states){
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
static void screen_update_settings_about(){
	menu_display(&Font_6x8, message->about);
	screen_driver_SetCursor_WriteString(message->about_brand, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	screen_driver_SetCursor_WriteString(message->about_product, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_SetCursor_WriteString(message->about_version, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
}

// The current selected menu part
void screen_update_settings(uint8_t current_select){
    uint8_t select_states[AMOUNT_OF_SETTINGS] = {0};
    select_current_state(select_states, AMOUNT_OF_SETTINGS, current_select);

	screen_driver_Fill(Black);
	if (current_select >= SETT_START_MENU && current_select <= SETT_BRIGHTNESS){
		screen_update_global_settings1(select_states);
	}
	else if (current_select >= SETT_MIDI_THRU && current_select <= CHANNEL_FILTER){
		screen_update_global_settings2(select_states);
	}
	else if (current_select >= FT1 && current_select <= FT16){
		screen_update_midi_filter(select_states);
	}
	else if (current_select == ABOUT){
		screen_update_settings_about();
	}
	screen_driver_Line(0, LINE_4_VERT, 127, LINE_4_VERT, White);
	screen_driver_SetCursor_WriteString(message->save_instruction, Font_6x8, White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	screen_driver_UpdateScreen();
}




// Save portion
static void saving_settings_ui(){
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




static void midi_filter_update_menu(TIM_HandleTypeDef *timer,
                                    uint16_t *filtered_channels,
									uint8_t current_select,
                                    uint8_t select_changed)
{
    uint8_t channel_index = (uint8_t)(current_select - FT1);
    uint8_t bit_value = (uint8_t)((*filtered_channels >> channel_index) & 1U);
    utils_counter_change(timer, &bit_value, 0, 1, select_changed, 1, WRAP);
    if (bit_value)
        *filtered_channels |=  (uint16_t)(1U << channel_index);
    else
        *filtered_channels &= ~(uint16_t)(1U << channel_index);
}




void settings_update_menu(TIM_HandleTypeDef * timer3,
                          TIM_HandleTypeDef * timer4,
                          uint8_t * old_menu,
						  uint8_t * current_select){

	static uint8_t old_select = 0;
	uint8_t contrast_index = calculate_contrast_index(settings_data.brightness);
	settings_data_struct old_settings_data = settings_data;


	uint8_t menu_changed = (*old_menu != SETTINGS);
	utils_counter_change(timer3, current_select, 0, AMOUNT_OF_SETTINGS-1, menu_changed, 1, WRAP);

	// Compute whether the selection changed before the switch
	uint8_t select_changed = (old_select != * current_select);
	switch (* current_select) {
		// Global section
		case SETT_START_MENU:
			utils_counter_change(timer4, &settings_data.start_menu, 0, AMOUNT_OF_MENUS-1, select_changed, 1, WRAP);
			break;

		case SETT_SEND_TO_USB:
			utils_counter_change(timer4, &settings_data.send_to_usb, USB_MIDI_OFF, USB_MIDI_SEND, select_changed, 1, WRAP);
			break;

		case SETT_BRIGHTNESS:
		   utils_counter_change(timer4, &contrast_index, 0, 9, select_changed, 1, NO_WRAP);
			static const uint8_t contrast_values[10] = {0x39,0x53,0x6D,0x87,0xA1,0xBB,0xD5,0xEF,0xF9,0xFF};
			settings_data.brightness = contrast_values[contrast_index];
			if (old_settings_data.brightness != settings_data.brightness) {
				screen_driver_SetContrast(settings_data.brightness);
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
			midi_filter_update_menu(timer4, &settings_data.filtered_channels, *current_select, select_changed);
		    break;
		}
	}



	if(debounce_button(GPIOB, Btn1_Pin, NULL, 10)){
		saving_settings_ui();
	}

    if (menu_check_for_updates(menu_changed,  &old_settings_data, &settings_data,
          sizeof settings_data, current_select, &old_select)) {
        osThreadFlagsSet(display_updateHandle, FLAG_SETTINGS);
    }
    old_select  = *current_select;
    *old_menu   = SETTINGS;
}


