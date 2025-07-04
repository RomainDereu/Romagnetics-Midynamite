/*
 * text.h
 *
 *  Created on: Jun 30, 2025
 *      Author: Romain Dereu
 */

#ifndef TEXT_H_
#define TEXT_H_

// Message structure that centralizes all user-facing UI strings
typedef struct {
    // Menu Titles
    const char *send_midi_tempo;
    const char *target;
    const char *settings_modify;
    const char *settings_transpose;
    const char *global_settings;
    const char *about;
    const char *midi_modify;


    // Channel Modify
    const char *change_midi_channel;
    const char *to_channel;
    const char *midi_modify_select;
    const char *midi_change;
    const char *midi_split;
    const char *split_point;
    const char *send_low_to_ch;
    const char *send_high_to_ch;

    // Velocity
    const char *velocity;
    const char *change;
    const char *fixed;
    const char *change_velocity;
    const char *fixed_velocity;

    // Transpose
    const char *type;
    const char *pitch_shift;
    const char *transpose;
    const char *mode;
    const char *na;

    // Modes
    const char *ionian;
    const char *dorian;
    const char *phrygian;
    const char *lydian;
    const char *mixolydian;
    const char *aeolian;
    const char *locrian;

    // MIDI out choices
    const char *send_to;
    const char *midi_channel_1;
    const char *midi_channel_2;
    const char *midi_channel_1_2;

    // Start menu
    const char *start_menu;
    const char *start_menu_tempo;
    const char *start_menu_modify;
    const char *start_menu_settings;

    // Contrast
    const char *contrast;
    const char *contrast_levels[10];

    // About
    const char *about_brand;
    const char *about_product;
    const char *about_version;

    // Saving messages
    const char *save_instruction;
    const char *saving;
    const char *saved;

    // On/Off
    const char *on;
    const char *off;

    // MIDI Tempo
    const char *bpm;

	//Midi notes names
    const char **midi_note_names;


} Message;

// Global pointer to access the centralized text messages
extern const Message *message;

#endif /* TEXT_H_ */
