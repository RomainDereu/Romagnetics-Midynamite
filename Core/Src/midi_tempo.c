/*
 * midi_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "midi_tempo.h"
#include "cmsis_os.h"


void send_midi_to_midi_out(UART_HandleTypeDef huart_ptr, uint32_t *tempo_click_rate_ptr){
	  uint8_t clock_send_tempo[3]  = {0xf8, 0x00, 0x00};
	  HAL_UART_Transmit(&huart_ptr, clock_send_tempo, 3, 1000);
	  //Adjusting the tempo if needed
	  if (TIM2->ARR != *tempo_click_rate_ptr){
		  TIM2->ARR = *tempo_click_rate_ptr;
	  }
	  osDelay(1);
}



void mt_press_btn3(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, screen_driver_Font_t * font){
	//Clock start and starting the timer
	uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
	HAL_UART_Transmit(uart, clock_start, 3, 1000);
	HAL_TIM_Base_Start_IT(timer);
	//Screen update
	screen_driver_SetCursor(30, 100);
	screen_driver_WriteString("Tempo On   ", *font , White);
	screen_driver_UpdateScreen();
}



void mt_press_btn4(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, screen_driver_Font_t * font){
	//Stopping the timer and sending stop message
	HAL_TIM_Base_Stop_IT(timer);
	uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};
	HAL_UART_Transmit(uart, clock_stop, 3, 1000);
	//Screen update
	screen_driver_SetCursor(30, 80);
	screen_driver_WriteString("Tempo Off   ", *font, White);
	screen_driver_UpdateScreen();
	HAL_Delay(1000);
}
