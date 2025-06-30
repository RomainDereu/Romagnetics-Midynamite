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

#include "screen_driver.h"
#include "screen_driver_fonts.h"

#include "midi_tempo.h"
#include "cmsis_os.h"
#include "menu.h"
#include "main.h"
#include "utils.h"

#include "text.h"
extern const Message * message;

extern osThreadId display_updateHandle;

//Midi messages constants
const uint8_t clock_send_tempo[3]  = {0xf8, 0x00, 0x00};
const uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
const uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};







void screen_update_midi_tempo(midi_tempo_data_struct * midi_tempo_data){
   	  screen_driver_Fill(Black);
	  //Menu
	  menu_display(&Font_6x8, message->send_midi_tempo);
	  //Vertical line
	  screen_driver_Line(64, 10, 64, 64, White);
	  //Horizontal line
	  screen_driver_Line(0, 40, 64, 40, White);

	  //Send to Midi Out and / or Out 2
      screen_driver_SetCursor(0, 15);
      screen_driver_WriteString(message->target, Font_6x8 , White);

      screen_driver_SetCursor(0, 25);
      if(midi_tempo_data->send_channels == MIDI_OUT_1){
      screen_driver_WriteString(message->midi_channel_1, Font_6x8 , White);
      }
      else if(midi_tempo_data->send_channels == MIDI_OUT_2){
      screen_driver_WriteString(message->midi_channel_2, Font_6x8 , White);
      }
      else if(midi_tempo_data->send_channels == MIDI_OUT_1_2){
      screen_driver_WriteString(message->midi_channel_1_2, Font_6x8 , White);
      }

      //Stop/Sending status
      screen_driver_SetCursor(15, 46);

      if(midi_tempo_data->currently_sending==0){
    	  screen_driver_WriteString(message->off, Font_11x18 , White);
      }
      else if (midi_tempo_data->currently_sending==1){
    	  screen_driver_WriteString(message->on, Font_11x18 , White);
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
	  screen_driver_WriteString(message->bpm, Font_6x8, White);

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
		osThreadFlagsSet(display_updateHandle, 0x01);
	}
}

void midi_tempo_update_menu(TIM_HandleTypeDef * timer3,
							TIM_HandleTypeDef * timer4,
                            midi_tempo_data_struct * midi_tempo_data,
							uint8_t * old_menu){
	uint8_t menu_changed = (*old_menu != MIDI_TEMPO);
	midi_tempo_data_struct old_midi_tempo_information = * midi_tempo_data;


	utils_counter_change(timer3, &(midi_tempo_data->send_channels), MIDI_OUT_1, MIDI_OUT_1_2, menu_changed);
	midi_tempo_value_counter(timer4, midi_tempo_data, menu_changed);

	*old_menu = MIDI_TEMPO;

	//Updating in case of change
	if(old_midi_tempo_information.current_tempo != midi_tempo_data->current_tempo ||
	   old_midi_tempo_information.currently_sending != midi_tempo_data->currently_sending ||
	   old_midi_tempo_information.send_channels != midi_tempo_data->send_channels||
	   menu_changed == 1){
	   osThreadFlagsSet(display_updateHandle, 0x01);
	}

}

void midi_tempo_value_counter(TIM_HandleTypeDef * timer,
                               midi_tempo_data_struct * midi_tempo_data,
                               uint8_t menu_changed) {

    static uint32_t old_tempo;

    if (menu_changed == 0) {

        uint8_t Btn2State = HAL_GPIO_ReadPin(GPIOB, Btn2_Pin);
        uint8_t change_value = (Btn2State == 0) ? 10 : 1;

        int32_t timer_count = __HAL_TIM_GET_COUNTER(timer);
        int32_t delta = timer_count - ENCODER_CENTER;

        if (delta >= ENCODER_THRESHOLD) {
            midi_tempo_data->current_tempo += change_value;
            __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
        }
        else if (delta <= -ENCODER_THRESHOLD) {
            midi_tempo_data->current_tempo -= change_value;
            __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
        }

        if (midi_tempo_data->current_tempo < 30) {
            midi_tempo_data->current_tempo = 30;
        } else if (midi_tempo_data->current_tempo > 300) {
            midi_tempo_data->current_tempo = 300;
        }
    }

    if (old_tempo != midi_tempo_data->current_tempo || menu_changed == 1) {
        old_tempo = midi_tempo_data->current_tempo;
    }

    if (menu_changed == 1) {
        __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
    }
}
