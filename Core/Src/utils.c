/*
 * utils.c
 *
 *  Created on: Jun 21, 2025
 *      Author: Astaa
 */

#include "utils.h"
#include "cmsis_os.h"

extern osThreadId display_updateHandle;


void utils_counter_change(TIM_HandleTypeDef * timer,
							   uint32_t * data_to_change,
							   uint8_t bottom_value,
							   uint8_t max_value,
							   uint8_t menu_changed){
	static uint32_t old_value;
    if (menu_changed == 0) {

		int32_t timer_count = __HAL_TIM_GET_COUNTER(timer);
		int32_t delta = timer_count - ENCODER_CENTER;

        if (delta >= ENCODER_THRESHOLD) {
        	* data_to_change += 1;
			__HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
        }

		else if (delta <= -ENCODER_THRESHOLD) {
			* data_to_change -= 1;
			__HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
		}

	  //checking if the values are out of bounds
        if (* data_to_change > max_value) {
        	* data_to_change = bottom_value;
        } else if (* data_to_change < bottom_value) {
        	* data_to_change = max_value;
        }

        if (old_value != * data_to_change) {
        	old_value = * data_to_change;
            osThreadFlagsSet(display_updateHandle, 0x01);
        }
    }
	if (menu_changed == 1) {
		__HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
	}
}
