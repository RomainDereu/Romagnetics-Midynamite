/*
 * menu.h
 *
 *  Created on: Feb 22, 2025
 *      Author: Astaa
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "main.h"
#include "screen_driver.h"

void menu_display(const screen_driver_Font_t * font, char (* message)[30]);

void menu_change(TIM_HandleTypeDef * timer, uint8_t * current_menu);

#endif /* INC_MENU_H_ */
