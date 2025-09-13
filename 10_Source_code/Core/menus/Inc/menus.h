/*
 * menus.h
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */

#ifndef INC_MENUS_H_
#define INC_MENUS_H_

#define TEXT_(m) ((const char*)(message->m))

// =====================
// Data structs
// =====================

typedef struct {
    ui_elem_type_t type;
    uint8_t        save_item;     // save_field_t index (fits in uint8_t in your layout)
    const char    *text;          // TEXT_(...) or UI_CHOICE(...)
    ui_font_t      font;
    int16_t        x;
    int16_t        y;
    uint8_t        ctrl_group_id; // CTRL_* visibility gate
} ui_element;

typedef struct {
    menu_list_t           menu_id;
    const ui_element     *elements;
    size_t                count;
} menu_ui_information;


// =====================
// Struct exposure
// =====================

extern const ui_element tempo_ui_elements[];
extern const size_t     tempo_ui_count;

extern const ui_element modify_ui_elements[];
extern const size_t     modify_ui_count;

extern const ui_element transpose_ui_elements[];
extern const size_t     transpose_ui_count;

extern const ui_element settings_ui_elements[];
extern const size_t     settings_ui_count;

#endif /* INC_MENUS_H_ */
