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

//All the tempo variables are set for a tempo of 60 but are dynamically changed in code
uint32_t old_tempo;
//Saving a copy of the latest value of the
uint32_t old_timer = 0;
//Midi messages constants
const uint8_t clock_send_tempo[3]  = {0xf8, 0x00, 0x00};
const uint8_t clock_start[3] = {0xfa, 0x00, 0x00};
const uint8_t clock_stop[3]  = {0xfc, 0x00, 0x00};


void screen_update_midi_tempo(struct midi_tempo_data_struct * midi_tempo_data){

	  //Menu
	  char message_midi_tempo[30] = "Send Midi Tempo              ";
	  menu_display(&Font_6x8, &message_midi_tempo);
	  //separation line
	  screen_driver_Line(64, 10, 64, 64, White);
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
      //On/Off status
      screen_driver_SetCursor(0, 40);
      char sending_print[10] = "Stopped   ";
      char no_sending_print[10] = "Sending   ";

      if(midi_tempo_data->currently_sending==0){
    	  screen_driver_WriteString(&sending_print[0], Font_6x8 , White);
      }
      else if (midi_tempo_data->currently_sending==1){
    	  screen_driver_WriteString(&no_sending_print[0], Font_6x8 , White);
      }
      screen_driver_UpdateScreen();

}


void send_midi_tempo_out(UART_HandleTypeDef huart_ptr, uint32_t current_tempo){
	  uint32_t tempo_click_rate = 6000000/(current_tempo*24);
	  HAL_UART_Transmit(&huart_ptr, clock_send_tempo, 3, 1000);
	  //Adjusting the tempo if needed
	  if (TIM2->ARR != tempo_click_rate){
		  TIM2->ARR = tempo_click_rate;
	  }
}


//Interrupter method. Do not add delay
void mt_start_stop(UART_HandleTypeDef * uart,
		           TIM_HandleTypeDef * timer,
				   struct midi_tempo_data_struct * midi_tempo_data_ptr){
	if(midi_tempo_data_ptr->currently_sending == 0){
		//Clock start and starting the timer
		HAL_UART_Transmit(uart, clock_start, 3, 1000);
		HAL_TIM_Base_Start_IT(timer);
		midi_tempo_data_ptr->currently_sending = 1;
		screen_update_midi_tempo(midi_tempo_data_ptr);
	}

	else if(midi_tempo_data_ptr->currently_sending == 1){
		//Stopping the timer and sending stop message
		HAL_TIM_Base_Stop_IT(timer);
		HAL_UART_Transmit(uart, clock_stop, 3, 1000);
		midi_tempo_data_ptr->currently_sending = 0;
		screen_update_midi_tempo(midi_tempo_data_ptr);
	}

	osDelay(10);
}



void midi_tempo_counter(TIM_HandleTypeDef * timer, struct midi_tempo_data_struct * midi_tempo_data){
	  //IF pressed, button 2 multiplies the tempo change by 10
	  uint8_t Btn2State = !HAL_GPIO_ReadPin(GPIOB, Btn2_Pin);
	  uint8_t change_value = 1;
	  if (Btn2State == 1){
		  change_value = 10;
		  }
	  //Checking if the timer has changed
      uint32_t new_timer = __HAL_TIM_GET_COUNTER(timer);
	  if (new_timer > old_timer && old_timer != 0){
		  midi_tempo_data->current_tempo+= change_value;
		  }
	  else if(new_timer < old_timer){
		  midi_tempo_data->current_tempo-= change_value;
	  }

	  //checking if the values are out of bounds
	  if (midi_tempo_data->current_tempo > 60000  || midi_tempo_data->current_tempo < 30)
	  {
		  midi_tempo_data->current_tempo = 30;
	  }
	  if (midi_tempo_data->current_tempo > 300)
	  {
		  midi_tempo_data->current_tempo = 300;
	  }

	  __HAL_TIM_SET_COUNTER(timer, midi_tempo_data->current_tempo);

	  if (old_tempo != midi_tempo_data->current_tempo){
		  //updating the screen if a new value appears
		  screen_update_midi_tempo(midi_tempo_data);
		  old_tempo = midi_tempo_data->current_tempo;
	  }
	  old_timer = __HAL_TIM_GET_COUNTER(timer);
}


