/*
 * text.h
 *
 *  Created on: Jun 30, 2025
 *      Author: Romain Dereu
 */

#ifndef TEXT_H_
#define TEXT_H_

// Central struct for all UI strings and selectable options
typedef struct {
    // Menu Titles
    const char *send_midi_tempo;
    const char *target;
    const char *settings_modify;
    const char *settings_transpose;
    const char *global_settings_1;
    const char *global_settings_2;

    const char *about;
    const char *midi_modify;
    const char *midi_transpose;

    // Settings
    const char *MIDI_Thru;
    const char *USB_Thru;
    const char *MIDI_Filter;

    const char *X_equals_ignore_channel;


    // Channel Modify
    const char *to_channel;
    const char *midi_modify_select;
    const char *split;
    const char *low_to_ch;
    const char *high_to_ch;

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
    const char *send_base_note;
    const char *interval;
    const char *root_note;

    const char *scale;
    const char *send_base;
    const char *shift_by;
    const char *semitones;

    // Modes
    const char *ionian;
    const char *dorian;
    const char *phrygian;
    const char *lydian;
    const char *mixolydian;
    const char *mixo;
    const char *aeolian;
    const char *locrian;

    // Intervals
    const char *octave_dn;
    const char *sixth_dn;
    const char *fifth_dn;
    const char *fourth_dn;
    const char *third_dn;
    const char *third_up;
    const char *fourth_up;
    const char *fifth_up;
    const char *sixth_up;
    const char *octave_up;

    // MIDI out choices
    const char *send_to;
    const char *midi_channel_1;
    const char *midi_channel_2;
    const char *midi_channel_1_2;
    const char *split_1_2;

    // Start Menu
    const char *start_menu;
    const char *tempo;
    const char *modify;
    const char *settings;

    // Contrast
    const char *contrast;
    const char *contrast_levels[10];

    // About
    const char *about_brand;
    const char *about_product;
    const char *about_version;

    // Saving
    const char *save_instruction;
    const char *saving;
    const char *saved;

    // Booleans
    const char *on;
    const char *off;
    const char *yes;
    const char *no;

    // MIDI Tempo
    const char *bpm;

    //USB Midi
    const char *usb_midi;


    //Upgrade mode
    const char *upgrade_mode;

    // Note names
    const char *midi_note_names[128];
    const char *twelve_notes_names[12];

    // All selectable dropdowns and grouped options
    struct {
        const char *change_split[2];
        const char *change_fixed[2];
        const char *midi_outs[4];
        const char *transpose_modes[2];
        const char *scales[7];
        const char *intervals[10];
        const char *no_yes[2];
        const char *off_on[2];
        const char *usb_receive_send[2];
    } choices;

} Message;

// Global access to UI strings and options
extern const Message *message;

#endif /* TEXT_H_ */
