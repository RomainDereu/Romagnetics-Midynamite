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

#define SETTINGS_FIRST_GLOBAL1  SETTINGS_START_MENU
#define SETTINGS_LAST_GLOBAL1   SETTINGS_BRIGHTNESS

#define SETTINGS_FIRST_GLOBAL2  SETTINGS_MIDI_THRU
#define SETTINGS_LAST_GLOBAL2   SETTINGS_CHANNEL_FILTER

#define SETTINGS_FIRST_FILTER   SETTINGS_FILTERED_CHANNELS


extern const Message *message;



uint8_t get_settings_items_count(void) {
    return build_select_states(UI_GROUP_SETTINGS, 0, NULL, 0) + 1; // +1 for "About"
}


// Settings Section
static void screen_update_global_settings1(uint8_t *select_states){
	menu_display(&Font_6x8, message->global_settings_1);
	uint8_t idx = SETTINGS_FIRST_GLOBAL1 - SETTINGS_START_MENU;

	// Start Menu
	screen_driver_SetCursor_WriteString(message->start_menu, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	const char *start_menu_options[] = {
		message->tempo,
		message->modify,
		message->transpose,
		message->settings,
	};
	screen_driver_underline_WriteString(start_menu_options[save_get(SETTINGS_START_MENU)], Font_6x8, White, 70, LINE_1_VERT, select_states[idx++]);

	// Send to USB
	screen_driver_SetCursor_WriteString(message->usb_midi, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_underline_WriteString(message->choices.usb_receive_send[save_get(SETTINGS_SEND_USB)], Font_6x8, White, 70, LINE_2_VERT, select_states[idx++]);

	// Contrast
	screen_driver_SetCursor_WriteString(message->contrast, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	uint8_t brightness_val = save_get(SETTINGS_BRIGHTNESS);
	if (brightness_val > 9) brightness_val = 9;
	screen_driver_underline_WriteString(message->contrast_levels[brightness_val], Font_6x8, White, 70, LINE_3_VERT, select_states[idx++]);
}


static void screen_update_global_settings2(uint8_t *select_states){
	menu_display(&Font_6x8, message->global_settings_2);
	uint8_t idx = SETTINGS_FIRST_GLOBAL2 - SETTINGS_START_MENU;

	// MIDI THRU
	screen_driver_SetCursor_WriteString(message->MIDI_Thru, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[save_get(SETTINGS_MIDI_THRU)], Font_6x8, White, 80, LINE_1_VERT, select_states[idx++]);

	// USB THRU
	screen_driver_SetCursor_WriteString(message->USB_Thru, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[save_get(SETTINGS_USB_THRU)], Font_6x8, White, 80, LINE_2_VERT, select_states[idx++]);

	// MIDI
	screen_driver_SetCursor_WriteString(message->MIDI_Filter, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	screen_driver_underline_WriteString(message->choices.off_on[save_get(SETTINGS_CHANNEL_FILTER)], Font_6x8, White, 80, LINE_3_VERT, select_states[idx++]);
}





static void screen_update_midi_filter(uint8_t *select_states)
{
    menu_display(&Font_6x8, message->MIDI_Filter);


    screen_driver_SetCursor_WriteString(message->X_equals_ignore_channel,
                                        Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);

    uint32_t mask = save_get_u32(SETTINGS_FILTERED_CHANNELS);
    for (uint8_t i = 0; i < 16; i++) {
        const char *label = (mask & ((uint32_t)1 << i))
                            ? "X"
                            : message->one_to_sixteen[i];

        uint8_t x = (uint8_t)(5 + 10 * (i % 8));
        uint8_t y = (i < 8) ? LINE_2_VERT : LINE_3_VERT;

        uint8_t base_idx = SETTINGS_FIRST_FILTER - SETTINGS_START_MENU;
        uint8_t underline = select_states[base_idx + i];

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

	uint8_t max_states = get_settings_items_count();
	uint8_t select_states[max_states];
	memset(select_states, 0, max_states);
	uint8_t current_select = ui_state_get(UI_SETTINGS_SELECT);

    (void)build_select_states(UI_GROUP_SETTINGS,
                              current_select,
                              select_states,
							  max_states);


	screen_driver_Fill(Black);

	if (current_select >= (SETTINGS_FIRST_GLOBAL1 - SETTINGS_START_MENU) &&
	    current_select <= (SETTINGS_LAST_GLOBAL1   - SETTINGS_START_MENU)) {
	    screen_update_global_settings1(select_states);
	}
	else if (current_select >= (SETTINGS_FIRST_GLOBAL2 - SETTINGS_START_MENU) &&
	         current_select <= (SETTINGS_LAST_GLOBAL2   - SETTINGS_START_MENU)) {
	    screen_update_global_settings2(select_states);
	}
	else if (current_select >= (SETTINGS_FIRST_FILTER - SETTINGS_START_MENU) &&
	         current_select <  (SETTINGS_FIRST_FILTER - SETTINGS_START_MENU) + 16) {
	    screen_update_midi_filter(select_states);
	}
	else {
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

	uint8_t max_states = get_settings_items_count();
	update_select(&current_select, 0, max_states - 1, 1, WRAP);
	ui_state_modify(UI_SETTINGS_SELECT, UI_MODIFY_SET, current_select);


    toggle_underline_items(UI_GROUP_SETTINGS, current_select);

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


