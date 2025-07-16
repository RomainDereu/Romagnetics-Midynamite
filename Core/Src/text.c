/*
 * text.c
 *
 *  Created on: Jun 30, 2025
 *      Author: Romain Dereu
 */


#include "text.h"

static const char *midi_notes_array[128] = {
    "C-1",  "C#/Db-1", "D-1",  "D#/Eb-1", "E-1",  "F-1",  "F#/Gb-1", "G-1",  "G#/Ab-1", "A-1",  "A#/Bb-1", "B-1",
    "C0",   "C#/Db0",  "D0",   "D#/Eb0",  "E0",   "F0",   "F#/Gb0",  "G0",   "G#/Ab0",  "A0",   "A#/Bb0",  "B0",
    "C1",   "C#/Db1",  "D1",   "D#/Eb1",  "E1",   "F1",   "F#/Gb1",  "G1",   "G#/Ab1",  "A1",   "A#/Bb1",  "B1",
    "C2",   "C#/Db2",  "D2",   "D#/Eb2",  "E2",   "F2",   "F#/Gb2",  "G2",   "G#/Ab2",  "A2",   "A#/Bb2",  "B2",
    "C3",   "C#/Db3",  "D3",   "D#/Eb3",  "E3",   "F3",   "F#/Gb3",  "G3",   "G#/Ab3",  "A3",   "A#/Bb3",  "B3",
    "C4",   "C#/Db4",  "D4",   "D#/Eb4",  "E4",   "F4",   "F#/Gb4",  "G4",   "G#/Ab4",  "A4",   "A#/Bb4",  "B4",
    "C5",   "C#/Db5",  "D5",   "D#/Eb5",  "E5",   "F5",   "F#/Gb5",  "G5",   "G#/Ab5",  "A5",   "A#/Bb5",  "B5",
    "C6",   "C#/Db6",  "D6",   "D#/Eb6",  "E6",   "F6",   "F#/Gb6",  "G6",   "G#/Ab6",  "A6",   "A#/Bb6",  "B6",
    "C7",   "C#/Db7",  "D7",   "D#/Eb7",  "E7",   "F7",   "F#/Gb7",  "G7",   "G#/Ab7",  "A7",   "A#/Bb7",  "B7",
    "C8",   "C#/Db8",  "D8",   "D#/Eb8",  "E8",   "F8",   "F#/Gb8",  "G8",   "G#/Ab8",  "A8",   "A#/Bb8",  "B8",
    "C9",   "C#/Db9",  "D9",   "D#/Eb9",  "E9",   "F9",   "F#/Gb9",  "G9"
};


static const char *twelve_notes_array[12] = {
    "C",  "C#/Db", "D",  "D#/Eb", "E",  "F",  "F#/Gb", "G",  "G#/Ab", "A",  "A#/Bb", "B"
};


static const Message message_data = {
	//Menu text
    .send_midi_tempo = "Send Midi Tempo",
    .target = "Target:",
    .settings_modify = "Settings Midi Modify",
    .settings_transpose = "Settings Transpose",
    .global_settings = "Global Settings",
    .about = "About",
	.midi_modify = "Midi Modify",
	.midi_transpose = "Midi Transpose",

    // Channel Modify
	.to_channel = "To channel",
    .midi_modify_select = "Ch. Modify",
    .split = "Split",
	.low_to_ch = "Low to Ch.",
	.high_to_ch = "High to Ch.",

    // Velocity
    .velocity = "Velocity",
    .change = "Change",
    .fixed = "Fixed",
	.change_velocity = "Change velocity",
	.fixed_velocity = "Fixed velocity",

    // Transpose
    .type = "Type",
    .pitch_shift = "Pitch Shift",
    .transpose = "Transpose",
    .mode = "Mode",
    .na = "N/A",
	.send_base_note = "Send base note",
	.interval = "Interval",
	.root_note = "Root note",

	.scale = "Scale",
	//mode is already defined
	.send_base = "Send Base",
	.shift_by = "Shift by",
	.semitones = "Semitones",

    // Modes
    .ionian = "Ionian",
    .dorian = "Dorian",
    .phrygian = "Phrygian",
    .lydian = "Lydian",
    .mixolydian = "Mixolydian",
    .mixo = "Mixo.",
    .aeolian = "Aeolian",
    .locrian = "Locrian",

	//Intervals
    .octave_dn = "Oct Dn",
    .sixth_dn = "6th Dn",
    .fifth_dn = "5th Dn",
    .fourth_dn = "4th Dn",
    .third_dn = "3rd Dn",
    .third_up = "3rd Up",
    .fourth_up = "4th Up",
    .fifth_up = "5th Up",
    .sixth_up = "6th Up",
    .octave_up = "Oct Up",

    // MIDI out choices
    .send_to = "Send to",
    .midi_channel_1 = "Out",
    .midi_channel_2 = "Out 2",
    .midi_channel_1_2 = "Out 1 & 2",
    .split_1_2 = "Split 1 2",

    // Start menu
    .start_menu = "Start Menu",
    .tempo = "Tempo",
    .modify = "Modify",
    .settings = "Settings",
    //Transpose is defined elsewhere


    // Contrast
    .contrast = "Contrast",
    .contrast_levels = {
        " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10"
    },

    // About
    .about_brand = "Romagnetics",
    .about_product = "Midynamite",
    .about_version = "Version 1.0",

    // Saving
    .save_instruction = "Press Select to save settings",
    .saving = "Saving                       ",
    .saved = "Saved!                       ",

    //On Off
    .on = "ON",
	.off = "OFF",

	.yes = "Yes",
	.no = "No",


	//Midi Tempo
	.bpm = "BPM",

	//Midi notes names
    .midi_note_names = midi_notes_array,

	//One octave note names
    .twelve_notes_names = twelve_notes_array


};


const Message *message = &message_data;

// Define the actual Message_choices storage
static Message_choices _message_choices;
Message_choices *message_choices = &_message_choices;

void init_message_choices(void) {
    _message_choices.change_split[0] = message->change;
    _message_choices.change_split[1] = message->split;

    _message_choices.change_fixed[0] = message->change;
    _message_choices.change_fixed[1] = message->fixed;

    _message_choices.midi_outs[0] = 0;
    _message_choices.midi_outs[1] = message->midi_channel_1;
    _message_choices.midi_outs[2] = message->midi_channel_2;
    _message_choices.midi_outs[3] = message->midi_channel_1_2;
    _message_choices.midi_outs[4] = message->split_1_2;

    _message_choices.transpose_modes[0] = message->pitch_shift;
    _message_choices.transpose_modes[1] = message->transpose;

    _message_choices.scales[0] = message->ionian;
    _message_choices.scales[1] = message->dorian;
    _message_choices.scales[2] = message->phrygian;
    _message_choices.scales[3] = message->lydian;
    _message_choices.scales[4] = message->mixo;
    _message_choices.scales[5] = message->aeolian;
    _message_choices.scales[6] = message->locrian;

    _message_choices.intervals[0] = message->octave_dn;
    _message_choices.intervals[1] = message->sixth_dn;
    _message_choices.intervals[2] = message->fifth_dn;
    _message_choices.intervals[3] = message->fourth_dn;
    _message_choices.intervals[4] = message->third_dn;
    _message_choices.intervals[5] = message->third_up;
    _message_choices.intervals[6] = message->fourth_up;
    _message_choices.intervals[7] = message->fifth_up;
    _message_choices.intervals[8] = message->sixth_up;
    _message_choices.intervals[9] = message->octave_up;

    _message_choices.yes_no[0] = message->yes;
    _message_choices.yes_no[1] = message->no;
}
