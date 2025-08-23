/*
 * utils.h
 *
 *  Created on: Jun 21, 2025
 *      Author: Romain Dereu
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "stdio.h"
#include "memory_main.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"

#define LINE_1_VERT 15
#define LINE_2_VERT 25
#define LINE_3_VERT 35
#define LINE_4_VERT 45
#define BOTTOM_LINE_VERT LINE_4_VERT + 3


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

#define TEXT_LEFT_START 5


#define WRAP 0
#define NO_WRAP 1




//Making an array with the list of uarts that will be used
void list_of_UART_to_send_to(uint8_t send_channels,
	                       	 UART_HandleTypeDef **UART_list);




void update_counter_32(TIM_HandleTypeDef *timer,
                       save_field_t field,
                       uint8_t menu_changed,
                       uint8_t multiplier);

void update_counter(TIM_HandleTypeDef *timer,
                    save_field_t field,
                    uint8_t menu_changed,
                    uint8_t multiplier);






//Roro the bottom two functions will be deleted after the refactoring
void utils_counter_change_i32(TIM_HandleTypeDef * timer,
                              int32_t * data_to_change,
                              int32_t bottom_value,
                              int32_t max_value,
                              uint8_t menu_changed,
                              uint8_t multiplier,
                              uint8_t wrap_or_not);

void utils_counter_change(TIM_HandleTypeDef * timer,
		                   uint8_t * data_to_change,
						   int32_t bottom_value,
						   int32_t max_value,
						   uint8_t menu_changed,
						   uint8_t multiplier,
						   uint8_t wrap_or_not);

void utils_change_settings(uint8_t * data_to_change, int8_t bottom_value, int32_t max_value);






void all_notes_off(UART_HandleTypeDef *huart);
void panic_midi(UART_HandleTypeDef *huart1, UART_HandleTypeDef *huart2,
				GPIO_TypeDef *port,
				uint16_t pin1,
				uint16_t pin2);


void midi_display_on_off(uint8_t on_or_off, uint8_t bottom_line);


uint8_t handle_menu_toggle(GPIO_TypeDef *port,
		                   uint16_t pin1,
		                   uint16_t pin2);

uint8_t debounce_button(GPIO_TypeDef *port,
		                uint16_t      pin,
		                uint8_t     *prev_state,
		                uint32_t      db_ms);


void select_current_state(uint8_t *select_states,
                          uint8_t  amount,
                          uint8_t  current_select);


uint8_t menu_check_for_updates(uint8_t menu_changed, const void *old_data,
		                       const void *data_ptr, size_t sz,
							   uint8_t *old_select, uint8_t *current_select);


#endif /* SRC_UTILS_H_ */
