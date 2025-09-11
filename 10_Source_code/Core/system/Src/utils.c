/*
 * utils.c
 *
 *  Created on: Jun 21, 2025
 *      Author: Romain Dereu
 */
#include <string.h>
#include "cmsis_os.h"
#include "memory_main.h"
#include "_menu_controller.h"
#include "screen_driver.h"   //For setcontrast
#include "stm32f4xx_hal.h"   // HAL types (TIM, GPIO)
#include "text.h"
#include "utils.h"


extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;


void list_of_UART_to_send_to(uint8_t send_channels,
                           	 UART_HandleTypeDef **UART_list){
	extern UART_HandleTypeDef huart1;
	extern UART_HandleTypeDef huart2;

	if (send_channels == MIDI_OUT_1){
		UART_list[0] = &huart1;
		UART_list[1] = NULL;
	}

	else if (send_channels == MIDI_OUT_2){
		UART_list[0] = &huart2;
		UART_list[1] = NULL;
	}

	else if (send_channels == MIDI_OUT_1_2 || send_channels == MIDI_OUT_SPLIT){
		UART_list[0] = &huart1;
		UART_list[1] = &huart2;
	}

	else{
		UART_list[0] = NULL;
		UART_list[1] = NULL;
	}

}



/* ---------------------------
 * Encoder helpers
 * --------------------------- */

int8_t encoder_read_step(TIM_HandleTypeDef *timer) {
    int32_t delta = __HAL_TIM_GET_COUNTER(timer) - ENCODER_CENTER;
    if (delta <= -ENCODER_THRESHOLD) { __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER); return -1; }
    if (delta >=  ENCODER_THRESHOLD) { __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER); return +1; }
    return 0; // no step
}








void no_update(save_field_t field, uint8_t arg) {
    (void)field; (void)arg;
}

void update_value(save_field_t field, uint8_t multiplier)
{
    TIM_HandleTypeDef *timer = &htim4;

    uint8_t active_mult = 1;
    if (multiplier != 1) {
        uint8_t Btn2State = HAL_GPIO_ReadPin(GPIOB, Btn2_Pin);
        active_mult = (Btn2State == 0) ? multiplier : 1;
    }

    int8_t step = encoder_read_step(timer);
    if (step == 0) return;

    const int32_t delta = (int32_t)step * (int32_t)active_mult;

    // Get current, add delta (no wrap here)
    int32_t cur = (int32_t)save_get(field);
    int32_t next = cur + delta;

    // Let save layer apply the (single) wrap/clamp via SET
    if (u32_fields[field]) {
        (void)save_modify_u32(field, SAVE_MODIFY_SET, (uint32_t)next);
    } else {
        (void)save_modify_u8(field,  SAVE_MODIFY_SET, (uint8_t)next);
    }
}



void update_contrast(save_field_t f, uint8_t step) {
    update_value(f, step);
    screen_driver_UpdateContrast();
}



//Specific logic for the channel_filter
// Toggle one channel bit per call when a detent is seen
void update_channel_filter(save_field_t field, uint8_t bit_index)
{
    if (bit_index > 15) return;

    TIM_HandleTypeDef *timer4 = &htim4;
    int8_t step = encoder_read_step(timer4);
    if (step == 0) return;

    uint32_t mask = (uint32_t)save_get(field);
    mask ^= (1UL << bit_index);  // toggle exactly this bit
    (void)save_modify_u32(field, SAVE_MODIFY_SET, mask);
}







/* ---------------------------
 * Panic / MIDI helpers
 * --------------------------- */

// Panic on a single UART
void all_notes_off(UART_HandleTypeDef *huart) {
    uint8_t all_notes_off_msg[3];
    uint8_t reset_ctrl_msg[3];

    for (uint8_t channel = 0; channel < 16; channel++) {
        uint8_t status = 0xB0 | channel;

        // All Notes Off
        all_notes_off_msg[0] = status;
        all_notes_off_msg[1] = 123;  // Controller number for All Notes Off
        all_notes_off_msg[2] = 0;

        // Reset All Controllers
        reset_ctrl_msg[0] = status;
        reset_ctrl_msg[1] = 121;     // Controller number for Reset All Controllers
        reset_ctrl_msg[2] = 0;

        HAL_UART_Transmit(huart, all_notes_off_msg, 3, 1000);
        HAL_UART_Transmit(huart, reset_ctrl_msg, 3, 1000);
    }
}

// Panic on both UART1 and UART2
void panic_midi(UART_HandleTypeDef *huart1,
			    UART_HandleTypeDef *huart2,
				GPIO_TypeDef *port,
				uint16_t pin1,
				uint16_t pin2) {

	if (HAL_GPIO_ReadPin(port, pin1) == GPIO_PIN_RESET &&
	    HAL_GPIO_ReadPin(port, pin2) == GPIO_PIN_RESET) {

    osDelay(300); // Debounce delay

    if (HAL_GPIO_ReadPin(port, pin1) == GPIO_PIN_RESET &&
        HAL_GPIO_ReadPin(port, pin2) == GPIO_PIN_RESET) {

    	all_notes_off(huart1);
    	all_notes_off(huart2);
    }
}

}


uint8_t debounce_button(GPIO_TypeDef *port,
		                uint16_t      pin,
		                uint8_t     *prev_state,
		                uint32_t      db_ms)
{
    uint8_t cur = HAL_GPIO_ReadPin(port, pin);

    // If we have a prev_state pointer, only fire on high→low (prev==1 && cur==0).
    // Otherwise, fire on cur==0 immediately.
    if (cur == 0 && (!prev_state || *prev_state == 1)) {
        osDelay(db_ms);
        cur = HAL_GPIO_ReadPin(port, pin);
        if (cur == 0) {
            if (prev_state) *prev_state = 0;
            return 1;
        }
    }

    // remember state if tracking
    if (prev_state) *prev_state = cur;
    return 0;
}






/* ---------------------------
 * GUI helpers
 * --------------------------- */






//GUI function setting a flag on the currently selected item
void select_current_state(uint8_t *select_states,
                          uint8_t  amount,
                          uint8_t  current_select)
{
    for (uint8_t i = 0; i < amount; i++) select_states[i] = 0;
    if (current_select < amount) {
        select_states[current_select] = 1;
    }
}

//Checks for updates to a menu and refreshes the screen if needed
uint8_t menu_check_for_updates(
    const void *old_data,
    const void *data_ptr,
    size_t      sz,
    uint8_t    *old_select,
    uint8_t    *current_select)
{
    const uint8_t sel_changed  = (*old_select != *current_select);
    const uint8_t data_changed = (memcmp(old_data, data_ptr, sz) != 0);
    return (sel_changed || data_changed);
}


