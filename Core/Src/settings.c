/*
 * settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */

//List of current select
#define MM_CHANNEL_SELECT 0
#define MM_VELOCITY_SELECT 1
#define MT_TRANSPOSE_MODE 2
#define MT_SCALE 3
#define MT_MIDI_SEND 4
#define SETT_START_MENU 5
#define SETT_BRIGHTNESS 6
#define ABOUT 7
#define AMOUNT_OF_SETTINGS 8

#include "cmsis_os.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"
#include "main.h"
#include "menu.h"
#include "utils.h"
#include "settings.h"

extern osThreadId display_updateHandle;

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;
extern midi_transpose_data_struct midi_transpose_data;
extern settings_data_struct settings_data;

char settings_modify_message[30] = "Settings Midi Modify         ";
char settings_transpose_message[30] = "Settings Transpose           ";
char settings_global_message[30] = "Global Settings           ";
char settings_about_message[30] = "About                        ";

//Array with all the possible select values. Is being used to update the UI
uint8_t select_states[AMOUNT_OF_SETTINGS] = {0, 0, 0, 0, 0, 0, 0};

//Lines
uint8_t line_1_vert = 15;
uint8_t line_2_vert = 25;
uint8_t line_3_vert = 35;

//Midi Modify page
//Channel Modify
char midi_modify_select_message[] = "Ch. Modify";
char midi_change_message[] = "Change";
char midi_split_message[] = "Split";
char * midi_split_choices[] = {midi_change_message, midi_split_message};


//Velocity
char velocity_select_message[] = "Velocity";
char velocity_change_message[] = "Change";
char velocity_fixed_message[] = "Fixed";
char * velocity_choices[] = {velocity_change_message, velocity_fixed_message};

//Transpose section
char type_message[] = "Type";
char pitch_shift_message[] = "Pitch Shift";
char transpose_message[] = "Transpose";
char * transpose_mode_choices[] = {pitch_shift_message, transpose_message};

char mode_message[] = "Mode";
char na_message[] = "N/A";

char ionian_message[]     = "Ionian";
char dorian_message[]     = "Dorian";
char phrygian_message[]   = "Phrygian";
char lydian_message[]     = "Lydian";
char mixolydian_message[] = "Mixolydian";
char aeolian_message[]    = "Aeolian";
char locrian_message[]    = "Locrian";
char *scale_choices[7] = {
    ionian_message,
    dorian_message,
    phrygian_message,
    lydian_message,
    mixolydian_message,
    aeolian_message,
    locrian_message
};

char send_to_message[] = "Send to";
char midi_channel_1_message[] = "Out";
char midi_channel_2_message[] = "Out 2";
char midi_channel_1_2_message[] = "Out 1 & 2";
char  *send_to_midi_choices[] = {midi_channel_1_message, midi_channel_2_message, midi_channel_1_2_message};

//Settings Section
//Starting Menu
char start_menu_message[] = "Start Menu";
char midi_tempo_message[] = "Tempo";
char midi_modif_message[] = "Modify";
char midi_settings_message[] = "Settings";
char * start_menu_choices[] = {midi_tempo_message, midi_modif_message, midi_settings_message};

//Contrast
char contrast_message[9] = "Contrast";
uint8_t contrast_values[10] = {0x39, 0x53, 0x6D, 0x87, 0xA1, 0xBB, 0xD5, 0xEF, 0xF9, 0xFF};
char * contrast_level_labels[10] = {" 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10"};
uint8_t contrast_index;

//About Section
char about_pedal_brand_message[12] = "Romagnetics";
char about_pedal_pedal_message[11] = "Midynamite";
char current_version_message[12] = "Version 1.0";

//Save portion
char save_settings_message[30] = "Press Select to save settings";
char saving_print[30] = "Saving                       ";
char saved_print[30] = "Saved!                       ";

uint8_t current_select = 0;
uint8_t old_select = 0;




//The current selected menu part

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


	//Saving
	screen_driver_SetCursor_WriteString(save_settings_message, Font_6x8, White, 0, 56);


	screen_driver_UpdateScreen();
}

void screen_update_settings_midi_modify(){
	menu_display(&Font_6x8, &settings_modify_message);
	//Midi Mode
	screen_driver_SetCursor_WriteString(midi_modify_select_message, Font_6x8, White, 0, line_1_vert);
	screen_driver_underline_WriteString(midi_split_choices[midi_modify_data.change_or_split],
										Font_6x8, White, 80, line_1_vert, select_states[MM_CHANNEL_SELECT]);


	//Velocity
	screen_driver_SetCursor_WriteString(velocity_select_message, Font_6x8, White, 0, line_2_vert);
	screen_driver_underline_WriteString(velocity_choices[midi_modify_data.velocity_type],
										Font_6x8, White, 80, line_2_vert, select_states[MM_VELOCITY_SELECT]);


}

void screen_update_settings_midi_transpose(){

	menu_display(&Font_6x8, &settings_transpose_message);

	//Transpose Mode
	screen_driver_SetCursor_WriteString(type_message, Font_6x8, White, 0, line_1_vert);
	screen_driver_underline_WriteString(transpose_mode_choices[midi_transpose_data.transpose_type],
										Font_6x8, White, 60, line_1_vert, select_states[MT_TRANSPOSE_MODE]);

	//If the mode is Transpose, show the scale, otherwise N/A
	screen_driver_SetCursor_WriteString(mode_message, Font_6x8, White, 0, line_2_vert);


	if(midi_transpose_data.transpose_type == MIDI_TRANSPOSE_SCALED){
		screen_driver_underline_WriteString(scale_choices[midi_transpose_data.transpose_scale],
											Font_6x8, White, 60, line_2_vert, select_states[MT_SCALE]);
	}
	else{
		screen_driver_underline_WriteString(na_message, Font_6x8, White, 60, line_2_vert, select_states[MT_SCALE]);
	}


	//Send to midi part
	screen_driver_SetCursor_WriteString(send_to_message, Font_6x8, White, 0, line_3_vert);
	screen_driver_underline_WriteString(send_to_midi_choices[midi_transpose_data.send_channels], Font_6x8, White, 60, line_3_vert, select_states[MT_MIDI_SEND]);

}

void screen_update_global_settings(){
	menu_display(&Font_6x8, &settings_global_message);
    //Start Menu
	screen_driver_SetCursor_WriteString(start_menu_message, Font_6x8, White, 0, line_1_vert);
	screen_driver_underline_WriteString(start_menu_choices[settings_data.start_menu],
										Font_6x8, White, 80, line_1_vert, select_states[SETT_START_MENU]);
	//Contrast
	screen_driver_SetCursor_WriteString(contrast_message, Font_6x8, White, 0, line_2_vert);
	screen_driver_underline_WriteString(contrast_level_labels[contrast_index],
										Font_6x8, White, 80, line_2_vert, select_states[SETT_BRIGHTNESS]);

}

void screen_update_settings_about(){
	screen_driver_SetCursor_WriteString(about_pedal_brand_message, Font_6x8, White, 0, line_1_vert);
	screen_driver_SetCursor_WriteString(about_pedal_pedal_message, Font_6x8, White, 0, line_2_vert);
	screen_driver_SetCursor_WriteString(current_version_message, Font_6x8, White, 0, line_3_vert);
	menu_display(&Font_6x8, &settings_about_message);
}


void settings_update_menu(TIM_HandleTypeDef * timer3,
		                  TIM_HandleTypeDef * timer4,
						  uint8_t * old_menu){

	contrast_index = calculate_contrast_index(settings_data.brightness);

	midi_modify_data_struct old_modify_data = midi_modify_data;
	midi_transpose_data_struct old_midi_transpose_data = midi_transpose_data;
	settings_data_struct old_settings_data = settings_data;

	uint8_t menu_changed = (*old_menu != SETTINGS);
	utils_counter_change(timer3, &current_select, 0, AMOUNT_OF_SETTINGS-1, menu_changed);


	//Midi Modify section

	if (current_select == MM_CHANNEL_SELECT ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &midi_modify_data.change_or_split, 0, 1, select_changed);
	}
	else if (current_select == MM_VELOCITY_SELECT ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &midi_modify_data.velocity_type, 0, 1, select_changed);
	}

	//Transpose section
	else if (current_select == MT_TRANSPOSE_MODE ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &midi_transpose_data.transpose_type, 0, 1, select_changed);
	}

	else if (current_select == MT_SCALE ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &midi_transpose_data.transpose_scale, 0, AMOUNT_OF_MODES-1, select_changed);
	}

	else if (current_select == MT_MIDI_SEND ){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &midi_transpose_data.send_channels, 0, 2, select_changed);
	}


	else if (current_select == SETT_START_MENU){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &settings_data.start_menu, 0, AMOUNT_OF_MENUS-1, select_changed);
	}

	else if (current_select == SETT_BRIGHTNESS){
	uint8_t select_changed = (old_select != current_select);
	utils_counter_change(timer4, &contrast_index, 0, 9, select_changed);
	 if (contrast_index < 10) {
	        settings_data.brightness = contrast_values[contrast_index];
	        if (old_settings_data.brightness!= settings_data.brightness){
	        	screen_driver_SetContrast(settings_data.brightness);
	        }
	    }
	}


	//Selecting the current item being selected
	//First resetting everything
	for (uint8_t x=0; x < AMOUNT_OF_SETTINGS; x++){
		select_states[x] = 0;
	}
	select_states[current_select] = 1;

	saving_settings_ui();

	if(menu_changed || current_select!= old_select ||
			old_modify_data.change_or_split != midi_modify_data.change_or_split ||
			old_modify_data.velocity_type != midi_modify_data.velocity_type ||
			old_midi_transpose_data.transpose_type != midi_transpose_data.transpose_type ||
			old_midi_transpose_data.transpose_scale != midi_transpose_data.transpose_scale ||
			old_settings_data.start_menu != settings_data.start_menu ||
			old_settings_data.brightness != settings_data.brightness){
		osThreadFlagsSet(display_updateHandle, 0x08);
	}

	*old_menu = SETTINGS;
	old_select = current_select;

}


void saving_settings_ui(){
	uint8_t Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
	  if(Btn1State == 0)
		 {
			 //Debouncing
			 osDelay(10);
			 Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
			  if(Btn1State == 0)
				 {
				 //Saving the current configuration to the memory
				 screen_driver_SetCursor_WriteString(saving_print, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();

				 save_struct memory_to_be_saved = creating_save(&midi_tempo_data,
						 	 	 	 	 	 	 	 	 	 	&midi_modify_data,
																&midi_transpose_data,
																&settings_data);
				 store_settings(&memory_to_be_saved);

				 screen_driver_SetCursor_WriteString(saved_print, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();
				 osDelay(1000);
				 screen_driver_SetCursor_WriteString(save_settings_message, Font_6x8, White, 0, 56);
				 screen_driver_UpdateScreen();
				  }
	  }

}

uint8_t calculate_contrast_index(uint8_t brightness) {
    for (uint8_t i = 0; i < sizeof(contrast_values); i++) {
        if (contrast_values[i] == brightness) {
            return i;  // Found exact match
        }
    }
    // Default value is full brightness
    return 9;
}
