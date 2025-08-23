/*
 * midi_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "memory.h"

//under_here_header_checks
#include "midi_tempo.h"
#include "menu.h"
#include "main.h"
#include "midi_usb.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"


extern const Message * message;


void screen_update_midi_tempo(midi_tempo_data_struct * midi_tempo_data, uint8_t * current_select){
   	  screen_driver_Fill(Black);
	  //Menu
	  menu_display(&Font_6x8, message->send_midi_tempo);
	  //Vertical line
	  screen_driver_Line(64, 10, 64, 64, White);
	  //Horizontal line
	  screen_driver_Line(0, 40, 64, 40, White);

	  uint8_t select_states[AMOUNT_OF_TEMPO_ITEMS] = {0};
	  select_current_state(select_states, AMOUNT_OF_TEMPO_ITEMS, *current_select);


 	  //Tempo
	  char tempo_print[4];
	  char tempo_number[4];
	  itoa(midi_tempo_data->current_tempo, tempo_number, 10);
	  sprintf(tempo_print, tempo_number);
	  screen_driver_underline_WriteString(tempo_print, Font_16x24, White, 80, 20, select_states[TEMPO_PRINT]);
	  screen_driver_SetCursor_WriteString(message->bpm, Font_6x8, White, 80, 48);



	  //Send to Midi Out and / or Out 2
      screen_driver_SetCursor_WriteString(message->target, Font_6x8 , White, TEXT_LEFT_START, 15);
      char message_midi_out[10];
      switch (midi_tempo_data->send_to_midi_out) {
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

      if(midi_tempo_data->currently_sending==0){
    	  screen_driver_WriteString(message->off, Font_11x18 , White);
      }
      else if (midi_tempo_data->currently_sending==1){
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

void mt_start_stop(TIM_HandleTypeDef *timer, midi_tempo_data_struct *midi_tempo_data) {
	uint8_t clock_start = 0xFA;
	uint8_t clock_stop  = 0xfC;

	static UART_HandleTypeDef *UART_list_tempo[2];
	list_of_UART_to_send_to(midi_tempo_data->send_to_midi_out, UART_list_tempo);

    // Stop clock
    if (midi_tempo_data->currently_sending == 0) {
        HAL_TIM_Base_Stop_IT(timer);

        for (int i = 0; i < 2; i++) {
            if (UART_list_tempo[i] != NULL) {
                HAL_UART_Transmit(UART_list_tempo[i], &clock_stop, 1, 1000);
            }
        }
        send_usb_midi_message(&clock_stop, 1);
    }
    // Start clock
    else if (midi_tempo_data->currently_sending == 1) {
        for (int i = 0; i < 2; i++) {
            if (UART_list_tempo[i] != NULL) {
                HAL_UART_Transmit(UART_list_tempo[i], &clock_start, 1, 1000);
            }
        }
        send_usb_midi_message(&clock_start, 1);

        HAL_TIM_Base_Start_IT(timer);
        list_of_UART_to_send_to(midi_tempo_data->send_to_midi_out, UART_list_tempo);
    }
}

void midi_tempo_update_menu(TIM_HandleTypeDef * timer3,
							TIM_HandleTypeDef * timer4,
                            midi_tempo_data_struct * midi_tempo_data,
							uint8_t * old_menu,
							uint8_t * current_select,
							osThreadId_t * display_updateHandle){
	static uint8_t old_select = 0;
	uint8_t select_changed = (old_select != * current_select);
	midi_tempo_data_struct old_midi_tempo_data = * midi_tempo_data;
	uint8_t menu_changed = (*old_menu != MIDI_TEMPO);

	utils_counter_change(timer3, current_select, 0, 1, menu_changed, 1, WRAP);
	switch (* current_select) {
		case 0:
			utils_counter_change_i32(timer4, &(midi_tempo_data->current_tempo), 30, 300, select_changed, 10, NO_WRAP);
			break;
		case 1:
			utils_counter_change(timer4, &(midi_tempo_data->send_to_midi_out), MIDI_OUT_1, MIDI_OUT_1_2, select_changed, 1, WRAP);
			break;
	}


	if(old_midi_tempo_data.current_tempo != midi_tempo_data->current_tempo){
		midi_tempo_data->tempo_click_rate = 6000000 / (midi_tempo_data->current_tempo * 24);
	}

    if (menu_check_for_updates( menu_changed, &old_midi_tempo_data, midi_tempo_data, sizeof *midi_tempo_data,
          current_select, &old_select  )) {
        osThreadFlagsSet(display_updateHandle, FLAG_TEMPO);
    }
    old_select  = * current_select;
    *old_menu   = MIDI_TEMPO;
}
