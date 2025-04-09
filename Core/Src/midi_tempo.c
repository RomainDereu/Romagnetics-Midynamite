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
uint32_t tempo = 60;
uint32_t old_tempo = 0;
extern uint32_t tempo_counter;
extern uint32_t tempo_click_rate;

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
void mt_press_btn3(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, const screen_driver_Font_t * font){
	//Clock start and starting the timer
		uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
		HAL_UART_Transmit(uart, clock_start, 3, 1000);
		HAL_TIM_Base_Start_IT(timer);
	//Screen update
	screen_driver_SetCursor(30, 50);
	screen_driver_WriteString("Tempo On   ", *font , White); // @suppress("Symbol is not resolved")
	screen_driver_UpdateScreen();
}


//Interrupter method. Do not add delay
void mt_press_btn4(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, const screen_driver_Font_t * font){
	//Stopping the timer and sending stop message
		HAL_TIM_Base_Stop_IT(timer);
		uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};
		HAL_UART_Transmit(uart, clock_stop, 3, 1000);
    //Screen update
	screen_driver_SetCursor(30, 50);
	screen_driver_WriteString("Tempo Off   ", *font, White);
	screen_driver_UpdateScreen();
}


//Font is 16x24
void midi_tempo_counter(TIM_HandleTypeDef * timer, const screen_driver_Font_t * font){
	  tempo_counter = __HAL_TIM_GET_COUNTER(timer);
	  tempo = tempo_counter / 4;
	  tempo_click_rate = 600000/(tempo*24);
	  if (tempo_counter > 60000  || tempo_counter < 120)
	  {
	    __HAL_TIM_SET_COUNTER(timer,120);
	    tempo =30;
	    tempo_click_rate = 208;
	  }
	  if (tempo_counter > 1200)
	  {
	    __HAL_TIM_SET_COUNTER(timer,1200);
	    tempo =300;
	    tempo_click_rate = 2083;
	  }

	  if (old_tempo != tempo){
		  //updating the screen if a new value appears
		  char number_print[3];
		  itoa(tempo ,number_print,10);
		  //blank spaces are added to delete any remaining numbers on the screen
		  char fullmessage[7];
		  sprintf(fullmessage, "%s   ", number_print);
		  screen_driver_SetCursor(48, 30);
		  screen_driver_WriteString(fullmessage, *font, White);
		  screen_driver_UpdateScreen();
		  old_tempo = tempo;
	  }
}


