/*
 * utils.h
 *
 *  Created on: Jun 21, 2025
 *      Author: Romain Dereu
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "stdio.h"
#include "main.h"

#endif /* SRC_UTILS_H_ */



void utils_counter_change(TIM_HandleTypeDef * timer,
		                   uint8_t * data_to_change,
						   uint8_t bottom_value,
						   uint8_t max_value,
						   uint8_t menu_changed);


void screen_update_settings();
