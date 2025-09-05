/*
 * midi_modify.h
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */

#ifndef INC_MIDI_MODIFY_H_
#define INC_MIDI_MODIFY_H_

#include "cmsis_os.h"
#include "memory_main.h"
#include "screen_driver.h"




//midi_modify_menu
void midi_modify_update_menu();
void screen_update_midi_modify();


//midi transpose menu
void midi_transpose_update_menu();
void screen_update_midi_transpose();


#endif /* INC_MIDI_MODIFY_H_ */
