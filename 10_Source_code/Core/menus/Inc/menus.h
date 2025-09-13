/*
 * menus.h
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */

#ifndef INC_MENUS_H_
#define INC_MENUS_H_

#define TEXT_(m) ((const char*)(message->m))


//Individual menu updates
void ui_code_midi_tempo();
void screen_update_midi_tempo();

void ui_code_midi_modify();
void screen_update_midi_modify();

void ui_code_midi_transpose();
void screen_update_midi_transpose();

void ui_code_settings();
void screen_update_settings();


//Menu helpers
void screen_update_menu(uint32_t flag);
void ui_code_menu();
void saving_settings_ui();
void update_contrast_ui();



//Thread related
void menu_change_check(void);
void start_stop_pressed();
void midi_display_on_off(uint8_t on_or_off, uint8_t bottom_line);



#endif /* INC_MENUS_H_ */
