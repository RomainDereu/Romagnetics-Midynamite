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
	if (idx == SAVE_STATE_BUSY || idx > 9) idx = 9;    // clamp 0..9
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

static void screen_update_midi_filter(uint8_t *select_states){
    menu_display(&Font_6x8, message->MIDI_Filter);
    screen_driver_SetCursor_WriteString(message->X_equals_ignore_channel, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);

    // Read the 32-bit mask safely
    uint32_t mask = save_get_u32(SAVE_SETTINGS_FILTERED_CHANNELS);
    if (mask == SAVE_STATE_BUSY) mask = 0;  // avoid rendering everything as 'X' on contention

    for (uint8_t i = 0; i < 16; i++) {
        char label[3];  // "16" + '\0' fits

        // Bit = 1 → blocked → show "X". Bit = 0 → allowed → show number
        if ((mask & (1UL << i)) != 0UL) {
            strcpy(label, "X");
        } else {
            snprintf(label, sizeof(label), "%u", (unsigned)(i + 1));
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
void screen_update_settings(){

    uint8_t select_states[AMOUNT_OF_SETTINGS_ITEMS] = {0};
	uint8_t current_select = ui_state_get(UI_SETTINGS_SELECT);
    select_current_state(select_states, AMOUNT_OF_SETTINGS_ITEMS, current_select);

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




static void midi_filter_update_menu(TIM_HandleTypeDef *timer,
                                    uint32_t *filtered_channels,
                                    uint8_t * current_select,
                                    uint8_t select_changed)
{
    uint8_t channel_index = (uint8_t)(*current_select - FT1);
    uint8_t bit_value = (uint8_t)((*filtered_channels >> channel_index) & 1U);

    update_select(timer, &bit_value, 0, 1, select_changed, 1, WRAP);

    if (bit_value)
        *filtered_channels |=  (uint32_t)(1UL << channel_index);
    else
        *filtered_channels &= ~(uint32_t)(1UL << channel_index);
}





void settings_update_menu(TIM_HandleTypeDef * timer3,
                          TIM_HandleTypeDef * timer4,
						  osThreadId_t * display_updateHandle){

	settings_data_struct old_settings_data = save_snapshot_settings();


	static uint8_t old_select = 0;

	uint8_t current_select = ui_state_get(UI_SETTINGS_SELECT);
	uint8_t old_menu = ui_state_get(UI_OLD_MENU);
	uint8_t menu_changed = (old_menu != SETTINGS);
	update_select(timer3, &current_select, 0, AMOUNT_OF_SETTINGS_ITEMS - 1, menu_changed, 1, WRAP);
	ui_state_modify(UI_SETTINGS_SELECT, UI_MODIFY_SET, current_select);
	uint8_t select_changed = (old_select != current_select);


	switch (current_select) {
		// Global section
		case SETT_START_MENU:
			update_counter(timer4, SAVE_SETTINGS_START_MENU, select_changed, 1);
			break;

		case SETT_SEND_TO_USB:
			update_counter(timer4, SAVE_SETTINGS_SEND_USB, select_changed, 1);
			break;

		case SETT_BRIGHTNESS:
			   // Let the helper update the stored index in memory
			    update_counter(timer4, SAVE_SETTINGS_BRIGHTNESS, select_changed, 1);

			    // Read back the updated index and apply to hardware
			    uint8_t idx2 = save_get(SAVE_SETTINGS_BRIGHTNESS);
			    if (idx2 == SAVE_STATE_BUSY || idx2 > 9) idx2 = 9;

			    static const uint8_t contrast_values[10] =
			        {0x39,0x53,0x6D,0x87,0xA1,0xBB,0xD5,0xEF,0xF9,0xFF};
			    uint8_t new_contrast = contrast_values[idx2];

			    if (old_settings_data.brightness != idx2) {
			        screen_driver_SetContrast(new_contrast);
			    }
			    break;
		case SETT_MIDI_THRU:
			update_counter(timer4, SAVE_SETTINGS_MIDI_THRU, select_changed, 1);
			break;
		case SETT_USB_THRU:
			update_counter(timer4, SAVE_SETTINGS_USB_THRU, select_changed, 1);
			break;
		case CHANNEL_FILTER:
			update_counter(timer4, SAVE_SETTINGS_CHANNEL_FILTER, select_changed, 1);
			break;

		case FT1: case FT2: case FT3: case FT4:
		case FT5: case FT6: case FT7: case FT8:
		case FT9: case FT10: case FT11: case FT12:
		case FT13: case FT14: case FT15: case FT16: {
		    uint32_t filtered_channels = save_get_u32(SAVE_SETTINGS_FILTERED_CHANNELS);
		    if (filtered_channels == SAVE_STATE_BUSY) {
		        break;
		    }
		    midi_filter_update_menu(timer4, &filtered_channels, &current_select, select_changed);
		    save_modify_u32(SAVE_SETTINGS_FILTERED_CHANNELS, SAVE_MODIFY_SET, filtered_channels);
		    break;
		}
	}
	if(debounce_button(GPIOB, Btn1_Pin, NULL, 10)){
		saving_settings_ui();
	}

	settings_data_struct new_settings_data = save_snapshot_settings();
    if (menu_check_for_updates(menu_changed,  &old_settings_data, &new_settings_data,
          sizeof new_settings_data, &current_select, &old_select)) {
        osThreadFlagsSet(display_updateHandle, FLAG_SETTINGS);
    }

    old_select  = current_select;
}


