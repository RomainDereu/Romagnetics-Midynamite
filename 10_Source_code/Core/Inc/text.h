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
    const char *midi_transpose;


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

    // Start menu
    const char *start_menu;
    const char *tempo;
    const char *modify;
    const char *settings;
    //transpose is defined elsewhere


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

    // Booleans
    const char *on;
    const char *off;

    const char *yes;
    const char *no;

    // MIDI Tempo
    const char *bpm;

	//Midi notes names
    const char **midi_note_names;

	//12 notes names
    const char **twelve_notes_names;


} Message;



// Message structure that centralizes all user-facing UI strings
typedef struct {
    const char *change_split[2];
    const char *change_fixed[2];
    const char *midi_outs[5];
    const char *transpose_modes[2];
    const char *scales[7];
    const char *intervals[10];
    const char *no_yes[2];
} Message_choices;

extern const Message *message;
extern Message_choices *message_choices;

void init_message_choices(void);

#endif /* TEXT_H_ */
