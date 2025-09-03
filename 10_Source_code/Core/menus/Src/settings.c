/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */
#include <string.h>
#include <stdio.h>


#include "memory_ui_state.h"
#include "memory_main.h"

//under_here_header_checks
#include "screen_driver.h"
#include "screen_driver_fonts.h"


#include "menu.h"
#include "utils.h"
#include "settings.h"
#include "text.h"
#include "threads.h"


extern const Message *message;


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
	screen_driver_underline_WriteString(start_menu_options[save_get(SAVE_SETTINGS_START_MENU)], Font_6x8, White, 70, LINE_1_VERT, select_states[SETT_START_MENU]);

	// Send to USB
	screen_driver_SetCursor_WriteString(message->usb_midi, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_underline_WriteString(message->choices.usb_receive_send[save_get(SAVE_SETTINGS_SEND_USB)], Font_6x8, White, 70, LINE_2_VERT, select_states[SETT_SEND_TO_USB]);

	// Contrast
	screen_driver_SetCursor_WriteString(message->contrast, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	uint8_t idx = save_get(SAVE_SETTINGS_BRIGHTNESS);
	if (idx > 9) idx = 9;
	screen_driver_underline_WriteString(message->contrast_levels[idx],
	                                    Font_6x8, White, 70, LINE_3_VERT,
	                                    select_states[SETT_BRIGHTNESS]);
}


static void screen_update_global_settings2(uint8_t *select_states){
	menu_display(&Font_6x8, message->global_settings_2);

	// MIDI THRU
	screen_driver_SetCursor_WriteString(message->MIDI_Thru, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[save_get(SAVE_SETTINGS_MIDI_THRU)], Font_6x8, White, 80, LINE_1_VERT, select_states[SETT_MIDI_THRU]);

	// USB THRU
	screen_driver_SetCursor_WriteString(message->USB_Thru, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[save_get(SAVE_SETTINGS_USB_THRU)], Font_6x8, White, 80, LINE_2_VERT, select_states[SETT_USB_THRU]);

	// MIDI
	screen_driver_SetCursor_WriteString(message->MIDI_Filter, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[save_get(SAVE_SETTINGS_CHANNEL_FILTER)], Font_6x8, White, 80, LINE_3_VERT, select_states[CHANNEL_FILTER]);
}





static void screen_update_midi_filter(uint8_t *select_states)
{
    menu_display(&Font_6x8, message->MIDI_Filter);
    screen_driver_SetCursor_WriteString(message->X_equals_ignore_channel,
                                        Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);

    uint32_t mask = save_get_u32(SAVE_SETTINGS_FILTERED_CHANNELS);
    for (uint8_t i = 0; i < 16; i++) {
        const char *label = (mask & ((uint32_t)1 << i))
                            ? "X"
                            : message->one_to_sixteen[i];

        uint8_t x = (uint8_t)(5 + 10 * (i % 8));
        uint8_t y = (i < 8) ? LINE_2_VERT : LINE_3_VERT;

        uint8_t underline = 0;
        uint8_t idx = (uint8_t)(FT1 + i);
        if (idx < AMOUNT_OF_SETTINGS_ITEMS) underline = select_states[idx] ? 1 : 0;

        screen_driver_underline_WriteString(label, Font_6x8_2, White, x, y, underline);
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
void screen_update_settings(){

    uint8_t select_states[AMOUNT_OF_SETTINGS_ITEMS] = {0};
	uint8_t current_select = ui_state_get(UI_SETTINGS_SELECT);
    select_current_state(select_states, AMOUNT_OF_SETTINGS_ITEMS, current_select);

	screen_driver_Fill(Black);
	if (current_select <= SETT_BRIGHTNESS){
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

		save_struct memory_to_be_saved;
		memory_to_be_saved.midi_tempo_data     = save_snapshot_tempo();
		memory_to_be_saved.midi_modify_data    = save_snapshot_modify();
		memory_to_be_saved.midi_transpose_data = save_snapshot_transpose();
		memory_to_be_saved.settings_data       = save_snapshot_settings();
		memory_to_be_saved.check_data_validity = DATA_VALIDITY_CHECKSUM;

		store_settings(&memory_to_be_saved);

		screen_driver_SetCursor_WriteString(message->saved, Font_6x8, White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
		screen_driver_UpdateScreen();
		osDelay(1000);
		screen_driver_SetCursor_WriteString(message->save_instruction, Font_6x8, White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
		screen_driver_UpdateScreen();
}




void settings_update_menu(){

	settings_data_struct old_settings_data = save_snapshot_settings();
	static uint8_t old_select = 0;
	uint8_t current_select = ui_state_get(UI_SETTINGS_SELECT);

	update_select(&current_select, 0, AMOUNT_OF_SETTINGS_ITEMS - 1, 1, WRAP);
	ui_state_modify(UI_SETTINGS_SELECT, UI_MODIFY_SET, current_select);


	switch (current_select) {
		// Global section
		case SETT_START_MENU:
			update_value(SAVE_SETTINGS_START_MENU, 1);
			break;

		case SETT_SEND_TO_USB:
			update_value(SAVE_SETTINGS_SEND_USB, 1);
			break;

		case SETT_BRIGHTNESS:
			   // Let the helper update the stored index in memory
			    update_value(SAVE_SETTINGS_BRIGHTNESS, 1);
			    screen_driver_UpdateContrast();
			    break;
		case SETT_MIDI_THRU:
			update_value(SAVE_SETTINGS_MIDI_THRU, 1);
			break;
		case SETT_USB_THRU:
			update_value(SAVE_SETTINGS_USB_THRU, 1);
			break;
		case CHANNEL_FILTER:
			update_value(SAVE_SETTINGS_CHANNEL_FILTER, 1);
			break;

		case FT1: case FT2: case FT3: case FT4:
		case FT5: case FT6: case FT7: case FT8:
		case FT9: case FT10: case FT11: case FT12:
		case FT13: case FT14: case FT15: case FT16: {
		    uint8_t channel_index = (uint8_t)(current_select - FT1);
		    update_channel_filter(SAVE_SETTINGS_FILTERED_CHANNELS, channel_index);
		    break;
		}
	}
	if(debounce_button(GPIOB, Btn1_Pin, NULL, 10)){
		saving_settings_ui();
	}

	settings_data_struct new_settings_data = save_snapshot_settings();
    if (menu_check_for_updates(&old_settings_data, &new_settings_data,
          sizeof new_settings_data, &current_select, &old_select)) {
    	threads_display_notify(FLAG_SETTINGS);
    }

    old_select  = current_select;
}


