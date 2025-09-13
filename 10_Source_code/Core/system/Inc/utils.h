/*
 * utils.h
 *
 *  Created on: Jun 21, 2025
 *      Author: Romain Dereu
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "memory_main.h"


#define NO_MULT 1
#define TEN_MULT 10
#define OCT_MULT 12

#define VEL_MIN 0
#define VEL_MAX 127

#define VOL_DEC_MAX -50
#define VOL_INC_MAX 50

#define NO_MIDI_CH 0
#define MIDI_CH_1 1
#define MIDI_CH_16 16

#define MIDDLE_C 60




//Making an array with the list of uarts that will be used
void list_of_UART_to_send_to(uint8_t send_channels,
	                       	 UART_HandleTypeDef **UART_list);


int8_t encoder_read_step(TIM_HandleTypeDef *timer);

void no_update(save_field_t field, uint8_t arg);
void update_value(save_field_t field, uint8_t multiplier);



void update_contrast(save_field_t f, uint8_t step);
void update_channel_filter(save_field_t field, uint8_t bit_index);



void all_notes_off(UART_HandleTypeDef *huart);
void panic_midi(UART_HandleTypeDef *huart1, UART_HandleTypeDef *huart2,
				GPIO_TypeDef *port,
				uint16_t pin1,
				uint16_t pin2);



uint8_t debounce_button(GPIO_TypeDef *port,
		                uint16_t      pin,
		                uint8_t     *prev_state,
		                uint32_t      db_ms);


void saving_settings_ui(void);


#endif /* SRC_UTILS_H_ */
