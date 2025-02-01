/*
 * debug.h
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */


#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_
#endif /* INC_DEBUG_H_ */

#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"


void debug_Testbutton(GPIO_TypeDef * button_GPIO_Port, uint16_t  button_pin,
		  	  	  	  	char message_idle[], char message_pressed[],
						int cursor_x, int cursor_y);


