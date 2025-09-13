/*
 * menus.c
 *
 *  Created on: Sep 13, 2025
 *      Author: Astaa
 */
#include "cmsis_os.h"
#include "main.h" //Timer
#include "menus.h"
#include "_menu_ui.h"
#include "_menu_controller.h"
#include "midi_tempo.h"
#include "screen_driver.h"
#include "stm32f4xx_hal.h"   // HAL types (TIM, GPIO)
#include "text.h"
#include "utils.h"


extern TIM_HandleTypeDef htim2;


//menu_ui.c
void screen_update_menu(uint32_t flag){

    uint8_t current = ui_state_get(CURRENT_MENU);
    if (flag & flag_for_menu((menu_list_t)current)) {
      switch (current) {
        case MIDI_TEMPO:     screen_update_midi_tempo();     break;
        case MIDI_MODIFY:    screen_update_midi_modify();    break;
        case MIDI_TRANSPOSE: screen_update_midi_transpose(); break;
        case SETTINGS:       screen_update_settings();       break;
      }
    }
}

void ui_code_menu(){
    uint8_t current = ui_state_get(CURRENT_MENU);
	switch (current) {
	  case MIDI_TEMPO:     ui_code_midi_tempo();     break;
	  case MIDI_MODIFY:    ui_code_midi_modify();    break;
	  case MIDI_TRANSPOSE: ui_code_midi_transpose(); break;
	  case SETTINGS:       ui_code_settings();       break;
      }
}

//menu_controller.c
void controller_update_menu(menu_list_t field){
    uint8_t current = ui_state_get(CURRENT_MENU);
	switch (current) {
	  case MIDI_TEMPO:     controller_update_tempo(field);break;
	  case MIDI_MODIFY:    ui_code_midi_modify(field);    break;
	  case MIDI_TRANSPOSE: ui_code_midi_transpose(field); break;
	  case SETTINGS:       ui_code_settings(field);       break;
      }
}

//save helper functions
void saving_settings_ui(){
    if (debounce_button(GPIOB, Btn1_Pin, NULL, 10)) {
		write_68(message->saving, TXT_LEFT, B_LINE);
		screen_driver_UpdateScreen();

		store_settings();

		write_68(message->saved, TXT_LEFT, B_LINE);
		screen_driver_UpdateScreen();
		osDelay(1000);
		write_68(message->save_instruction, TXT_LEFT, B_LINE);
		screen_driver_UpdateScreen();
    }
}


void update_contrast_ui() {
    screen_driver_UpdateContrast();
}


//menu_change functions
void menu_change_check(){
	 static uint8_t button_pressed = 0;
	  if(debounce_button(GPIOB, Btn4_Pin, &button_pressed, 50)){
		  ui_state_modify(CURRENT_MENU, UI_MODIFY_INCREMENT, 0);
	  }
}

static uint8_t handle_menu_toggle(GPIO_TypeDef *port, uint16_t pin1, uint16_t pin2)
{
    static uint8_t prev_s1 = 1;

    const uint8_t s1 = HAL_GPIO_ReadPin(port, pin1);
    const uint8_t s2 = HAL_GPIO_ReadPin(port, pin2);

    // Rising → falling on s1 while s2 is high
    if (s1 == 0 && prev_s1 == 1 && s2 == 1) {
        osDelay(100);
        // Re-read after debounce
        if (HAL_GPIO_ReadPin(port, pin1) == 0 && HAL_GPIO_ReadPin(port, pin2) == 1) {
            prev_s1 = 0;
            return 1;
        }
    }

    prev_s1 = s1;
    return 0;
}


// Unified “subpage toggle” used by MODIFY and TRANSPOSE
void toggle_subpage(menu_list_t field) {
  if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
    select_press_menu_change(field);  // resets select + rebuilds list
  }
}


void start_stop_pressed() {
	menu_list_t m = (menu_list_t)ui_state_get(CURRENT_MENU);
	save_field_t f = sending_field_for_menu(m);
	if (f != SAVE_FIELD_INVALID) {
	  save_modify_u8(f, SAVE_MODIFY_INCREMENT, 0);
	  if (m == MIDI_TEMPO) mt_start_stop(&htim2);
	  threads_display_notify(flag_for_menu(m));
	}
}


void midi_display_on_off(uint8_t on_or_off, uint8_t bottom_line){
	draw_line(92, 10, 92, bottom_line);
	uint8_t text_position = bottom_line/2;
    const char *text_print = message->off_on[on_or_off];
	write_1118(text_print, 95, text_position);
}




