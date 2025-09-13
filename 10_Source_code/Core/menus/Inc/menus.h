/*
 * menus.h
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */

#include "_menu_controller.h" // for menu_list_t

#ifndef INC_MENUS_H_
#define INC_MENUS_H_

#define TEXT_(m) ((const char*)(message->m))


//Individual menu updates
void controller_update_tempo();
void ui_code_tempo();
void screen_update_midi_tempo();

void controller_update_modify(menu_list_t field);
void ui_code_modify();
void screen_update_midi_modify();

void controller_update_transpose(menu_list_t field);
void ui_code_transpose();
void screen_update_midi_transpose();

void controller_update_settings();
void ui_code_settings();
void screen_update_settings();


//Menu helpers
void screen_update_menu(uint32_t flag);
void ui_code_menu();
void controller_update_menu(menu_list_t field);

void saving_settings_ui();
void update_contrast_ui();

void menu_change_check();
void toggle_subpage(menu_list_t field);



//Thread related
void menu_change_check(void);
void start_stop_pressed();
void midi_display_on_off(uint8_t on_or_off, uint8_t bottom_line);



#endif /* INC_MENUS_H_ */
