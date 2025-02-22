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
