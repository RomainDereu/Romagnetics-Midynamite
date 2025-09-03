/*
 * midi_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory_ui_state.h"
#include "memory_main.h"

//under_here_header_checks
#include "midi_tempo.h"
#include "menu.h"
#include "midi_usb.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "threads.h"
#include "utils.h"


extern const Message * message;


void screen_update_midi_tempo(){

	  uint8_t current_select = ui_state_get(UI_MIDI_TEMPO_SELECT);

	  uint8_t select_states[AMOUNT_OF_TEMPO_ITEMS] = {0};
	  (void)build_select_states(UI_GROUP_TEMPO, current_select,
	                              select_states, AMOUNT_OF_TEMPO_ITEMS);

   	  screen_driver_Fill(Black);
	  //Menu
	  menu_display(&Font_6x8, message->send_midi_tempo);
	  //Vertical line
	  screen_driver_Line(64, 10, 64, 64, White);
	  //Horizontal line
	  screen_driver_Line(0, 40, 64, 40, White);

 	  //Tempo
	  char tempo_print[4];
	  snprintf(tempo_print, sizeof tempo_print, "%lu",
	           (unsigned long)save_get_u32(MIDI_TEMPO_CURRENT_TEMPO));
	  screen_driver_underline_WriteString(tempo_print, Font_16x24, White, 80, 20, select_states[TEMPO_PRINT]);
	  screen_driver_SetCursor_WriteString(message->bpm, Font_6x8, White, 80, 48);



	  //Send to Midi Out and / or Out 2
      screen_driver_SetCursor_WriteString(message->target, Font_6x8 , White, TEXT_LEFT_START, 15);
      char message_midi_out[10];
      switch (save_get(MIDI_TEMPO_SEND_TO_MIDI_OUT)) {
        case MIDI_OUT_1:
          strcpy(message_midi_out, message->midi_channel_1);
          break;

        case MIDI_OUT_2:
          strcpy(message_midi_out, message->midi_channel_2);
          break;

        case MIDI_OUT_1_2:
          strcpy(message_midi_out, message->midi_channel_1_2);
          break;

        default:
          // Optionally handle invalid values
          strcpy(message_midi_out,  message->error);
          break;
      }

      screen_driver_underline_WriteString(message_midi_out, Font_6x8 , White, TEXT_LEFT_START, 25, select_states[MIDI_OUT_PRINT]);

      //Stop/Sending status
      screen_driver_SetCursor(15, 42);
      uint8_t currently_sending = save_get(MIDI_TEMPO_CURRENTLY_SENDING);

      if(currently_sending == 0){
    	  screen_driver_WriteString(message->off, Font_11x18 , White);
      }
      else if (currently_sending == 1){
    	  screen_driver_WriteString(message->on, Font_11x18 , White);
      }

      screen_driver_UpdateScreen();

}


void send_midi_tempo_out(int32_t tempo_click_rate, uint8_t send_to_midi_out){
	uint8_t clock_tick = 0xF8;

	static UART_HandleTypeDef *UART_list_tempo[2];
	list_of_UART_to_send_to(send_to_midi_out, UART_list_tempo);


    send_usb_midi_message(&clock_tick, 1);
    for (int i = 0; i < 2; i++) {
        if (UART_list_tempo[i] != NULL) {
            HAL_UART_Transmit(UART_list_tempo[i], &clock_tick, 1, 1000);
        	}
    	}
	TIM2->ARR = tempo_click_rate;
    }

void mt_start_stop(TIM_HandleTypeDef *timer) {
	uint8_t clock_start = 0xFA;
	uint8_t clock_stop  = 0xfC;

	static UART_HandleTypeDef *UART_list_tempo[2];
	uint8_t send_out_to = save_get(MIDI_TEMPO_SEND_TO_MIDI_OUT);
	list_of_UART_to_send_to(send_out_to, UART_list_tempo);

	uint8_t clock_sending = save_get(MIDI_TEMPO_CURRENTLY_SENDING);

    // Stop clock
    if (clock_sending == 0) {
        HAL_TIM_Base_Stop_IT(timer);

        for (int i = 0; i < 2; i++) {
            if (UART_list_tempo[i] != NULL) {
                HAL_UART_Transmit(UART_list_tempo[i], &clock_stop, 1, 1000);
            }
        }
        send_usb_midi_message(&clock_stop, 1);
    }
    // Start clock
    else if (clock_sending == 1) {
        for (int i = 0; i < 2; i++) {
            if (UART_list_tempo[i] != NULL) {
                HAL_UART_Transmit(UART_list_tempo[i], &clock_start, 1, 1000);
            }
        }
        send_usb_midi_message(&clock_start, 1);

        HAL_TIM_Base_Start_IT(timer);
    }
}

void midi_tempo_update_menu(){
	midi_tempo_data_struct old_midi_tempo_data = save_snapshot_tempo();

	static uint8_t old_select = 0;
	uint8_t current_select = ui_state_get(UI_MIDI_TEMPO_SELECT);
	update_select(&current_select, 0, 1, 1, WRAP);
	ui_state_modify(UI_MIDI_TEMPO_SELECT, UI_MODIFY_SET ,current_select);

    toggle_underline_items(UI_GROUP_TEMPO, current_select);

	uint32_t new_tempo = save_get_u32(MIDI_TEMPO_CURRENT_TEMPO);
	save_modify_u32(MIDI_TEMPO_TEMPO_CLICK_RATE, SAVE_MODIFY_SET, 6000000 / (new_tempo * 24));

	midi_tempo_data_struct new_midi_tempo_data = save_snapshot_tempo();
	uint8_t tempo_has_changed = menu_check_for_updates( &old_midi_tempo_data,
														&new_midi_tempo_data,
														sizeof new_midi_tempo_data,
														&current_select,
														&old_select );

    if (tempo_has_changed) {
    	threads_display_notify(FLAG_TEMPO);
    }
    old_select  = current_select;
}
