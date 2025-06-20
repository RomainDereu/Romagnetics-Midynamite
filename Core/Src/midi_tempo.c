/*
 * midi_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "screen_driver.h"
#include "screen_driver_fonts.h"

#include "midi_tempo.h"
#include "cmsis_os.h"
#include "menu.h"
#include "main.h"
#include <math.h>


extern osThreadId display_updateHandle;

//All the tempo variables are set for a tempo of 60 but are dynamically changed in code
uint32_t old_tempo;
uint32_t old_target;

//Saving a copy of the latest value of the select and value timers
uint32_t old_select_timer = 0;
uint32_t old_value_timer = 0;


static int32_t tempo_residual = 0;

//Midi messages constants
const uint8_t clock_send_tempo[3]  = {0xf8, 0x00, 0x00};
const uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
const uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};

//Messaqges
char message_midi_tempo_print[30] = "Send Midi Tempo              ";
char target_channel_print[7] = "Target:";

char midi_channel_1_print[9] = "Out      ";
char midi_channel_2_print[9] = "Out 2    ";
char midi_channel_1_2_print[9] = "Out 1 & 2";


char sending_print[10] = "Sending   ";
char stopped_print[10] = "Stopped   ";


void screen_update_midi_tempo(midi_tempo_data_struct * midi_tempo_data){

	  //Menu
	  menu_display(&Font_6x8, &message_midi_tempo_print);
	  //Vertical line
	  screen_driver_Line(64, 10, 64, 64, White);
	  //Horizontal line
	  screen_driver_Line(0, 45, 64, 45, White);

	  //Send to Midi Out and / or Out 2
      screen_driver_SetCursor(0, 20);
      screen_driver_WriteString(target_channel_print, Font_6x8 , White);
      screen_driver_SetCursor(0, 32);
      if(midi_tempo_data->send_channels == MIDI_OUT_1){
      screen_driver_WriteString(midi_channel_1_print, Font_6x8 , White);
      }
      else if(midi_tempo_data->send_channels == MIDI_OUT_2){
      screen_driver_WriteString(midi_channel_2_print, Font_6x8 , White);
      }
      else if(midi_tempo_data->send_channels == MIDI_OUT_1_2){
      screen_driver_WriteString(midi_channel_1_2_print, Font_6x8 , White);
      }

      //Stop/Sending status
      screen_driver_SetCursor(0, 55);

      if(midi_tempo_data->currently_sending==0){
    	  screen_driver_WriteString(stopped_print, Font_6x8 , White);
      }
      else if (midi_tempo_data->currently_sending==1){
    	  screen_driver_WriteString(sending_print, Font_6x8 , White);
      }
 	  //Tempo
	  char tempo_number[3];
	  itoa(midi_tempo_data->current_tempo ,tempo_number,10);
	  //blank spaces are added to delete any remaining numbers on the screen
	  char tempo_print[7];
	  sprintf(tempo_print, "%s   ", tempo_number);
	  screen_driver_SetCursor(80, 30);
	  screen_driver_WriteString(tempo_print, Font_16x24, White);
	  screen_driver_SetCursor(80, 55);
	  screen_driver_WriteString("BPM", Font_6x8, White);

      screen_driver_UpdateScreen();
}


void send_midi_tempo_out(UART_HandleTypeDef *UART_list[2], uint32_t current_tempo){
	  uint32_t tempo_click_rate = 6000000/(current_tempo*24);
	  if (UART_list[0] != NULL){
		  HAL_UART_Transmit(UART_list[0], clock_send_tempo, 3, 1000);
	  }
	  if (UART_list[1] != NULL){
		  HAL_UART_Transmit(UART_list[1], clock_send_tempo, 3, 1000);
	  }
	  //Adjusting the tempo if needed
	  if (TIM2->ARR != tempo_click_rate){
		  TIM2->ARR = tempo_click_rate;
	  }
}



void mt_start_stop(UART_HandleTypeDef *UART_list[2],
		           TIM_HandleTypeDef * timer,
				   midi_tempo_data_struct * midi_tempo_data){
	if(midi_tempo_data->currently_sending == 0){
		//Clock start and starting the timer
		if (UART_list[0] != NULL){
			HAL_UART_Transmit(UART_list[0], clock_start, 3, 1000);
		}
		if (UART_list[1] != NULL){
			HAL_UART_Transmit(UART_list[1], clock_start, 3, 1000);
		}

		HAL_TIM_Base_Start_IT(timer);
		midi_tempo_data->currently_sending = 1;
		//Updating the display in another thread
		osThreadFlagsSet(display_updateHandle, 0x01);
	}

	else if(midi_tempo_data->currently_sending == 1){
		//Stopping the timer and sending stop message
		HAL_TIM_Base_Stop_IT(timer);
		if (UART_list[0] != NULL){
			HAL_UART_Transmit(UART_list[0], clock_stop, 3, 1000);
		}
		if (UART_list[1] != NULL){
			HAL_UART_Transmit(UART_list[1], clock_stop, 3, 1000);
		}

		midi_tempo_data->currently_sending = 0;
		//Updating the display in another thread
		osThreadFlagsSet(display_updateHandle, 0x01);
	}
}



void midi_tempo_select_counter(TIM_HandleTypeDef * timer,
                               midi_tempo_data_struct * midi_tempo_data,
							   uint8_t menu_changed){

    uint32_t new_select_timer = __HAL_TIM_GET_COUNTER(timer);
    int32_t steps = (new_select_timer - old_select_timer) / 4;

    if (steps != 0 && menu_changed == 0) {
        midi_tempo_data->send_channels += steps;

	  //checking if the values are out of bounds
        if (midi_tempo_data->send_channels > MIDI_OUT_1_2) {
            midi_tempo_data->send_channels = MIDI_OUT_1;
        } else if (midi_tempo_data->send_channels < MIDI_OUT_1) {
            midi_tempo_data->send_channels = MIDI_OUT_1_2;
        }

        if (old_target != midi_tempo_data->send_channels || menu_changed == 1) {
            osThreadFlagsSet(display_updateHandle, 0x01);
            old_target = midi_tempo_data->send_channels;
        }

        __HAL_TIM_SET_COUNTER(timer, midi_tempo_data->send_channels);
        old_select_timer = __HAL_TIM_GET_COUNTER(timer);
    }
}

void midi_tempo_value_counter(TIM_HandleTypeDef * timer,
                               midi_tempo_data_struct * midi_tempo_data,
                               uint8_t menu_changed) {

	//Refresh refers to the changing of menu
    if (menu_changed == 0) {

        uint8_t Btn2State = !HAL_GPIO_ReadPin(GPIOB, Btn2_Pin);
        uint8_t change_value = (Btn2State == 1) ? 10 : 1;

        int32_t timer_count = __HAL_TIM_GET_COUNTER(timer);
        int32_t delta = timer_count - ENCODER_CENTER;

        tempo_residual += delta;
        int32_t steps = tempo_residual / TICKS_PER_STEP;
        tempo_residual %= TICKS_PER_STEP;

        if (steps != 0) {
            midi_tempo_data->current_tempo += steps * change_value;

            if (midi_tempo_data->current_tempo < 30) {
                midi_tempo_data->current_tempo = 30;
            } else if (midi_tempo_data->current_tempo > 300) {
                midi_tempo_data->current_tempo = 300;
            }
        }
    }

    // Update the display
    if (old_tempo != midi_tempo_data->current_tempo || menu_changed == 1) {
        osThreadFlagsSet(display_updateHandle, 0x01);
    }

    // Reset timer to match new center
    old_tempo = midi_tempo_data->current_tempo;
    __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);

}
