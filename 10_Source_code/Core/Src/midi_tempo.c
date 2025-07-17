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

#include "cmsis_os.h"
#include "midi_tempo.h"
#include "menu.h"
#include "main.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"

extern const Message * message;

extern osThreadId display_updateHandle;

static UART_HandleTypeDef *UART_list_tempo[2];

static uint8_t select_states[2] = {0};
static uint8_t current_select = 0;
static uint8_t old_select = 0;

//Midi messages constants
static const uint8_t clock_tick = 0xF8;
static const uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
static const uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};


void screen_update_midi_tempo(midi_tempo_data_struct * midi_tempo_data){
   	  screen_driver_Fill(Black);
	  //Menu
	  menu_display(&Font_6x8, message->send_midi_tempo);
	  //Vertical line
	  screen_driver_Line(64, 10, 64, 64, White);
	  //Horizontal line
	  screen_driver_Line(0, 40, 64, 40, White);


 	  //Tempo
	  char tempo_print[4];
	  char tempo_number[4];
	  itoa(midi_tempo_data->current_tempo, tempo_number, 10);
	  sprintf(tempo_print, tempo_number);
	  screen_driver_underline_WriteString(tempo_print, Font_16x24, White, 80, 20, select_states[0]);
	  screen_driver_SetCursor_WriteString(message->bpm, Font_6x8, White, 80, 48);



	  //Send to Midi Out and / or Out 2
      screen_driver_SetCursor_WriteString(message->target, Font_6x8 , White, TEXT_LEFT_START, 15);
      char message_midi_out[10];


      if(midi_tempo_data->send_to_midi_out == MIDI_OUT_1){
    	  strcpy(message_midi_out, message->midi_channel_1);
      }
      else if(midi_tempo_data->send_to_midi_out == MIDI_OUT_2){
    	  strcpy(message_midi_out, message->midi_channel_2);
      }
      else if(midi_tempo_data->send_to_midi_out == MIDI_OUT_1_2){
    	  strcpy(message_midi_out, message->midi_channel_1_2);
      }

      screen_driver_underline_WriteString(message_midi_out, Font_6x8 , White, TEXT_LEFT_START, 25, select_states[1]);

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


void send_midi_tempo_out(int32_t current_tempo){
    uint32_t tempo_click_rate = 6000000 / (current_tempo * 24);

    for (int i = 0; i < 2; i++) {
        if (UART_list_tempo[i] != NULL) {
            HAL_UART_Transmit(UART_list_tempo[i], &clock_tick, 1, 1000);
        }
    }

    if (TIM2->ARR != tempo_click_rate) {
        TIM2->ARR = tempo_click_rate;
    }
}

void mt_start_stop(TIM_HandleTypeDef *timer, midi_tempo_data_struct *midi_tempo_data) {
    // Stop clock
    if (midi_tempo_data->currently_sending == 0) {
        HAL_TIM_Base_Stop_IT(timer);

        for (int i = 0; i < 2; i++) {
            if (UART_list_tempo[i] != NULL) {
                HAL_UART_Transmit(UART_list_tempo[i], clock_stop, 3, 1000);
            }
        }
    }
    // Start clock
    else if (midi_tempo_data->currently_sending == 1) {
        for (int i = 0; i < 2; i++) {
            if (UART_list_tempo[i] != NULL) {
                HAL_UART_Transmit(UART_list_tempo[i], clock_start, 3, 1000);
            }
        }

        HAL_TIM_Base_Start_IT(timer);
        list_of_UART_to_send_to(midi_tempo_data->send_to_midi_out, UART_list_tempo);
    }
}

void midi_tempo_update_menu(TIM_HandleTypeDef * timer3,
							TIM_HandleTypeDef * timer4,
                            midi_tempo_data_struct * midi_tempo_data,
							uint8_t * old_menu){

	midi_tempo_data_struct old_midi_tempo_information = * midi_tempo_data;
	uint8_t menu_changed = (*old_menu != MIDI_TEMPO);

	utils_counter_change(timer3, &current_select, 0, 1, menu_changed, 1, WRAP);

	// Compute whether the selection changed before the switch
	uint8_t select_changed = (old_select != current_select);
	switch (current_select) {
		case 0:
			utils_counter_change_i32(timer4, &(midi_tempo_data->current_tempo), 30, 300, select_changed, 10, NO_WRAP);
			break;
		case 1:
			utils_counter_change(timer4, &(midi_tempo_data->send_to_midi_out), MIDI_OUT_1, MIDI_OUT_1_2, select_changed, 1, WRAP);
			break;
	}

	//Updating the select_states
	select_states[0] = 0;
	select_states[1] = 0;
	select_states[current_select] = 1;


	list_of_UART_to_send_to(midi_tempo_data->send_to_midi_out, UART_list_tempo);



	//Updating in case of change
	if(old_midi_tempo_information.current_tempo != midi_tempo_data->current_tempo ||
	   old_midi_tempo_information.currently_sending != midi_tempo_data->currently_sending ||
	   old_midi_tempo_information.send_to_midi_out != midi_tempo_data->send_to_midi_out||
	   menu_changed == 1 || current_select != old_select ){
	   osThreadFlagsSet(display_updateHandle, 0x01);
	}

	*old_menu = MIDI_TEMPO;
	old_select = current_select;
}
