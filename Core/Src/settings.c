/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */

// List of current select
#define MM_CHANNEL_SELECT    0
#define MM_VELOCITY_SELECT   1
#define MT_TRANSPOSE_MODE    2
#define MT_SCALE             3
#define MT_MIDI_SEND         4
#define SETT_START_MENU      5
#define SETT_BRIGHTNESS      6
#define ABOUT                7
#define AMOUNT_OF_SETTINGS   8

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
	if(current_select >= MM_CHANNEL_SELECT && current_select <= MM_VELOCITY_SELECT){
		screen_update_settings_midi_modify();
	}
	else if (current_select >= MT_TRANSPOSE_MODE && current_select <= MT_MIDI_SEND){
		screen_update_settings_midi_transpose();
	}
	else if (current_select >= SETT_START_MENU && current_select <= SETT_BRIGHTNESS){
		screen_update_global_settings();
	}
	else if (current_select == ABOUT){
		screen_update_settings_about();
	}
	screen_driver_SetCursor_WriteString(message->save_instruction, Font_6x8, White, 0, 56);
	screen_driver_UpdateScreen();
}

// Midi Modify page
void screen_update_settings_midi_modify(){
	menu_display(&Font_6x8, message->settings_modify);

	// Channel Modify
	screen_driver_SetCursor_WriteString(message->midi_modify_select, Font_6x8, White, 0, LINE_1_VERT);
	const char *split_choices[] = {message->midi_change, message->midi_split};
	screen_driver_underline_WriteString(split_choices[midi_modify_data.change_or_split], Font_6x8, White, 80, LINE_1_VERT, select_states[MM_CHANNEL_SELECT]);

	// Velocity
	screen_driver_SetCursor_WriteString(message->velocity, Font_6x8, White, 0, LINE_2_VERT);
	const char *velocity_choices[] = {message->change, message->fixed};
	screen_driver_underline_WriteString(velocity_choices[midi_modify_data.velocity_type], Font_6x8, White, 80, LINE_2_VERT, select_states[MM_VELOCITY_SELECT]);
}

// Transpose section
void screen_update_settings_midi_transpose(){
	menu_display(&Font_6x8, message->settings_transpose);

	// Transpose Mode
	screen_driver_SetCursor_WriteString(message->type, Font_6x8, White, 0, LINE_1_VERT);
	const char *transpose_modes[] = {message->pitch_shift, message->transpose};
	screen_driver_underline_WriteString(transpose_modes[midi_transpose_data.transpose_type], Font_6x8, White, 60, LINE_1_VERT, select_states[MT_TRANSPOSE_MODE]);

	// Scale or N/A
	screen_driver_SetCursor_WriteString(message->mode, Font_6x8, White, 0, LINE_2_VERT);
	if(midi_transpose_data.transpose_type == MIDI_TRANSPOSE_SCALED){
		const char *scales[] = {
			message->ionian, message->dorian, message->phrygian,
			message->lydian, message->mixolydian, message->aeolian, message->locrian
		};
		screen_driver_underline_WriteString(scales[midi_transpose_data.transpose_scale], Font_6x8, White, 60, LINE_2_VERT, select_states[MT_SCALE]);
	} else {
		screen_driver_underline_WriteString(message->na, Font_6x8, White, 60, LINE_2_VERT, select_states[MT_SCALE]);
	}

	// Send to midi part
	screen_driver_SetCursor_WriteString(message->send_to, Font_6x8, White, 0, LINE_3_VERT);
	const char *midi_outs[] = {message->midi_channel_1, message->midi_channel_2, message->midi_channel_1_2};
	screen_driver_underline_WriteString(midi_outs[midi_transpose_data.send_channels], Font_6x8, White, 60, LINE_3_VERT, select_states[MT_MIDI_SEND]);
}

// Settings Section
void screen_update_global_settings(){
	menu_display(&Font_6x8, message->global_settings);

	// Start Menu
	screen_driver_SetCursor_WriteString(message->start_menu, Font_6x8, White, 0, LINE_1_VERT);
	const char *start_menu_options[] = {
		message->start_menu_tempo,
		message->start_menu_modify,
		message->start_menu_settings
	};
	screen_driver_underline_WriteString(start_menu_options[settings_data.start_menu], Font_6x8, White, 80, LINE_1_VERT, select_states[SETT_START_MENU]);

	// Contrast
	screen_driver_SetCursor_WriteString(message->contrast, Font_6x8, White, 0, LINE_2_VERT);
	screen_driver_underline_WriteString(message->contrast_levels[contrast_index], Font_6x8, White, 80, LINE_2_VERT, select_states[SETT_BRIGHTNESS]);
}

// About Section
void screen_update_settings_about(){
	menu_display(&Font_6x8, message->about);
	screen_driver_SetCursor_WriteString(message->about_brand, Font_6x8, White, 0, LINE_1_VERT);
	screen_driver_SetCursor_WriteString(message->about_product, Font_6x8, White, 0, LINE_2_VERT);
	screen_driver_SetCursor_WriteString(message->about_version, Font_6x8, White, 0, LINE_3_VERT);
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
	utils_counter_change(timer3, &current_select, 0, AMOUNT_OF_SETTINGS-1, menu_changed);

	// Compute whether the selection changed before the switch
	uint8_t select_changed = (old_select != current_select);
	switch (current_select) {
		// Midi Modify section
		case MM_CHANNEL_SELECT:
			utils_counter_change(timer4, &midi_modify_data.change_or_split, 0, 1, select_changed);
			break;

		case MM_VELOCITY_SELECT:
			utils_counter_change(timer4, &midi_modify_data.velocity_type, 0, 1, select_changed);
			break;

		// Transpose section
		case MT_TRANSPOSE_MODE:
			utils_counter_change(timer4, &midi_transpose_data.transpose_type, 0, 1, select_changed);
			break;

		case MT_SCALE:
			utils_counter_change(timer4, &midi_transpose_data.transpose_scale, 0, AMOUNT_OF_MODES-1, select_changed);
			break;

		case MT_MIDI_SEND:
			utils_counter_change(timer4, &midi_transpose_data.send_channels, 0, 2, select_changed);
			break;

		// Global section
		case SETT_START_MENU:
			utils_counter_change(timer4, &settings_data.start_menu, 0, AMOUNT_OF_MENUS-1, select_changed);
			break;

		case SETT_BRIGHTNESS:
			utils_counter_change(timer4, &contrast_index, 0, 9, select_changed);
			if (contrast_index < 10) {
				settings_data.brightness = contrast_values[contrast_index];
				if (old_settings_data.brightness != settings_data.brightness) {
					screen_driver_SetContrast(settings_data.brightness);
				}
			}
			break;
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
		old_midi_transpose_data.transpose_type != midi_transpose_data.transpose_type ||
		old_midi_transpose_data.transpose_scale != midi_transpose_data.transpose_scale ||
		old_midi_transpose_data.send_channels != midi_transpose_data.send_channels ||
		old_settings_data.start_menu != settings_data.start_menu ||
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
			screen_driver_SetCursor_WriteString(message->saving, Font_6x8, White, 0, 56);
			screen_driver_UpdateScreen();

			save_struct memory_to_be_saved = creating_save(&midi_tempo_data,
			                                               &midi_modify_data,
			                                               &midi_transpose_data,
			                                               &settings_data);
			store_settings(&memory_to_be_saved);

			screen_driver_SetCursor_WriteString(message->saved, Font_6x8, White, 0, 56);
			screen_driver_UpdateScreen();
			osDelay(1000);
			screen_driver_SetCursor_WriteString(message->save_instruction, Font_6x8, White, 0, 56);
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
