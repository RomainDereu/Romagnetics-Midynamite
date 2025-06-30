/*
 * text.c
 *
 *  Created on: Jun 30, 2025
 *      Author: Romain Dereu
 */


#include "text.h"

static const Message message_data = {
	//Menu text
    .send_midi_tempo = "Send Midi Tempo",
    .target = "Target:",
    .settings_modify = "Settings Midi Modify",
    .settings_transpose = "Settings Transpose",
    .global_settings = "Global Settings",
    .about = "About",
	.midi_modify = "Midi Modify",
	.change_midi_channel = "Change Midi Channel",

    // Channel Modify
    .midi_modify_select = "Ch. Modify",
    .midi_change = "Change",
    .midi_split = "Split",

    // Velocity
    .velocity_select = "Velocity",
    .velocity_change = "Change",
    .velocity_fixed = "Fixed",

    // Transpose
    .type = "Type",
    .pitch_shift = "Pitch Shift",
    .transpose = "Transpose",
    .mode = "Mode",
    .na = "N/A",

    // Modes
    .ionian = "Ionian",
    .dorian = "Dorian",
    .phrygian = "Phrygian",
    .lydian = "Lydian",
    .mixolydian = "Mixolydian",
    .aeolian = "Aeolian",
    .locrian = "Locrian",

    // MIDI out choices
    .send_to = "Send to",
    .midi_channel_1 = "Out",
    .midi_channel_2 = "Out 2",
    .midi_channel_1_2 = "Out 1 & 2",

    // Start menu
    .start_menu = "Start Menu",
    .start_menu_tempo = "Tempo",
    .start_menu_modify = "Modify",
    .start_menu_settings = "Settings",

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

	//Midi Tempo
	.bpm = "BPM"
};

const Message *message = &message_data;
