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
#include "midi_tempo.h"
#include "cmsis_os.h"


//All the tempo variables are set for a tempo of 60 but are dynamically changed in code
uint32_t current_tempo;
uint32_t old_tempo;
extern struct midi_tempo_data_struct midi_tempo_data;



uint8_t start_stop_status = CURRENTLY_STOP;


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
void mt_start_stop(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, const screen_driver_Font_t * font){
	if(start_stop_status == CURRENTLY_STOP){
		//Clock start and starting the timer
		uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
		HAL_UART_Transmit(uart, clock_start, 3, 1000);
		HAL_TIM_Base_Start_IT(timer);
		//Screen update
		screen_driver_SetCursor(30, 50);
		screen_driver_WriteString("Tempo On   ", *font , White); // @suppress("Symbol is not resolved")
		screen_driver_UpdateScreen();
		start_stop_status = CURRENTLY_START;
	}

	else if(start_stop_status == CURRENTLY_START){
		//Stopping the timer and sending stop message
		HAL_TIM_Base_Stop_IT(timer);
		uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};
		HAL_UART_Transmit(uart, clock_stop, 3, 1000);
	    //Screen update
		screen_driver_SetCursor(30, 50);
		screen_driver_WriteString("Tempo Off   ", *font, White);
		screen_driver_UpdateScreen();
		start_stop_status = CURRENTLY_STOP;
	}

	osDelay(100);
}



//Font is 16x24
void midi_tempo_counter(TIM_HandleTypeDef * timer, const screen_driver_Font_t * font){
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
		  char number_print[3];
		  itoa(current_tempo ,number_print,10);
		  //blank spaces are added to delete any remaining numbers on the screen
		  char fullmessage[7];
		  sprintf(fullmessage, "%s   ", number_print);
		  screen_driver_SetCursor(48, 30);
		  screen_driver_WriteString(fullmessage, *font, White);
		  screen_driver_UpdateScreen();
		  old_tempo = current_tempo;
	  }
}


