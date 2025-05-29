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


//All the tempo variables are set for a tempo of 60 but are dynamically changed in code
uint32_t current_tempo;
uint32_t old_tempo;
extern struct midi_tempo_data_struct midi_tempo_data;

void screen_update_midi_tempo(struct midi_tempo_data_struct * midi_tempo_data){

	  //Menu
	  char message_midi_tempo[30] = "Send Midi Tempo              ";
	  menu_display(&Font_6x8, &message_midi_tempo);
	  //separation line
	  screen_driver_Line(64, 10, 64, 64, White);
 	  //Tempo
	  char tempo_number[3];
	  current_tempo = midi_tempo_data->tempo_counter/4;
	  itoa(current_tempo ,tempo_number,10);
	  //blank spaces are added to delete any remaining numbers on the screen
	  char tempo_print[7];
	  sprintf(tempo_print, "%s   ", tempo_number);
	  screen_driver_SetCursor(80, 30);
	  screen_driver_WriteString(tempo_print, Font_16x24, White);
	  screen_driver_SetCursor(80, 55);
	  screen_driver_WriteString("BPM", Font_6x8, White);
      //On/Off status
      screen_driver_SetCursor(0, 40);
      char sending_print[7] = "Stopped";
      char no_sending_print[7] = "Sending";

      if(midi_tempo_data->currently_sending==0){
    	  screen_driver_WriteString(sending_print, Font_6x8 , White);
      }
      else if (midi_tempo_data->currently_sending==1){
    	  screen_driver_WriteString(no_sending_print, Font_6x8 , White);
      }
      screen_driver_UpdateScreen();

}


void send_midi_to_midi_out(UART_HandleTypeDef huart_ptr, uint32_t *tempo_click_rate_ptr){
	  uint8_t clock_send_tempo[3]  = {0xf8, 0x00, 0x00};
	  HAL_UART_Transmit(&huart_ptr, clock_send_tempo, 3, 1000);
	  //Adjusting the tempo if needed
	  if (TIM2->ARR != *tempo_click_rate_ptr){
		  TIM2->ARR = *tempo_click_rate_ptr;
	  }
	  osDelay(1);
}


//Interrupter method. Do not add delay
void mt_start_stop(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer){
	if(midi_tempo_data.currently_sending == 0){
		//Clock start and starting the timer
		uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
		HAL_UART_Transmit(uart, clock_start, 3, 1000);
		HAL_TIM_Base_Start_IT(timer);
		//Screen update
		screen_update_midi_tempo(&midi_tempo_data);
		midi_tempo_data.currently_sending = 1;
	}

	else if(midi_tempo_data.currently_sending == 1){
		//Stopping the timer and sending stop message
		HAL_TIM_Base_Stop_IT(timer);
		uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};
		HAL_UART_Transmit(uart, clock_stop, 3, 1000);
	    //Screen update
		screen_update_midi_tempo(&midi_tempo_data);
		midi_tempo_data.currently_sending = 0;
	}

	osDelay(100);
}



void midi_tempo_counter(TIM_HandleTypeDef * timer){
	  midi_tempo_data.tempo_counter = __HAL_TIM_GET_COUNTER(timer);
	  current_tempo = midi_tempo_data.tempo_counter / 4;
	  midi_tempo_data.tempo_click_rate = 600000/(current_tempo*24);
	  if (midi_tempo_data.tempo_counter > 60000  || midi_tempo_data.tempo_counter < 120)
	  {
	    __HAL_TIM_SET_COUNTER(timer,120);
	    current_tempo =30;
	    midi_tempo_data.tempo_click_rate = 208;
	  }
	  if (midi_tempo_data.tempo_counter > 1200)
	  {
	    __HAL_TIM_SET_COUNTER(timer,1200);
	    current_tempo =300;
	    midi_tempo_data.tempo_click_rate = 2083;
	  }

	  if (old_tempo != current_tempo){
		  //updating the screen if a new value appears
		  screen_update_midi_tempo(&midi_tempo_data);
		  old_tempo = current_tempo;
	  }
}


