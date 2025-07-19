/*
 * text.c
 *
 *  Created on: Jun 30, 2025
 *      Author: Romain Dereu
 */

#include "text.h"

static const Message _message = {
    // Menu Titles
    .send_midi_tempo = "Send MIDI Tempo",
    .target = "Target",
    .settings_modify = "Settings Midi Modify",
    .settings_transpose = "Settings Transpose",
    .global_settings = "Global Settings",
    .about = "About",
    .midi_modify = "MIDI Modify",
    .midi_transpose = "MIDI Transpose",

    // Channel Modify
    .to_channel = "To Channel",
    .midi_modify_select = "Ch. Modify",
    .split = "Split",
    .low_to_ch = "Low to CH",
    .high_to_ch = "High to CH",

    // Velocity
    .velocity = "Velocity",
    .change = "Change",
    .fixed = "Fixed",
    .change_velocity = "Change Velocity",
    .fixed_velocity = "Fixed Velocity",

    // Transpose
    .type = "Type",
    .pitch_shift = "Pitch Shift",
    .transpose = "Transpose",
    .mode = "Mode",
    .na = "N/A",
    .send_base_note = "Send Base Note",
    .interval = "Interval",
    .root_note = "Root Note",
    .scale = "Scale",
    .send_base = "Send Base",
    .shift_by = "Shift By",
    .semitones = "Semitones",

    // Modes
    .ionian = "Ionian",
    .dorian = "Dorian",
    .phrygian = "Phrygian",
    .lydian = "Lydian",
    .mixolydian = "Mixolydian",
    .mixo = "Mixo",
    .aeolian = "Aeolian",
    .locrian = "Locrian",

    // Intervals
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

    // MIDI Out
    .send_to = "Send To",
    .midi_channel_1 = "Out",
    .midi_channel_2 = "Out 2",
    .midi_channel_1_2 = "OUT 1+2",
    .split_1_2 = "Split 1/2",

    // Start Menu
    .start_menu = "Start Menu",
    .tempo = "Tempo",
    .modify = "Modify",
    .settings = "Settings",

    // Contrast
    .contrast = "Contrast",
    .contrast_levels = {
        "10%", "20%", "30%", "40%", "50%",
        "60%", "70%", "80%", "90%", "100%"
    },

    // About
    .about_brand = "Romagnetics",
    .about_product = "Midynamite",
    .about_version = "v1.0.0",

    // Saving
    .save_instruction = "Press select to Save",
    .saving = "Saving...           ",
    .saved = "Saved!              ",

    // Booleans
    .on = "On",
    .off = "Off",
    .yes = "Yes",
    .no = "No",

    // MIDI Tempo
    .bpm = "BPM",

    //USB Midi
	.usb_midi = "USB Midi",

    // MIDI Note Names (C-1 to G9)
    .midi_note_names = {
        "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",
        "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
        "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
        "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
        "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
        "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
        "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
        "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
        "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
        "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8", "A8", "A#8", "B8",
        "C9", "C#9", "D9", "D#9", "E9", "F9", "F#9", "G9"
    },

    // 12 Notes (for scales, transposition etc.)
    .twelve_notes_names = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    },

    // Dropdowns and grouped choices
    .choices = {
        .change_split = { "Change", "Split" },
        .change_fixed = { "Change", "Fixed" },
        .midi_outs = {"Out 1", "Out 2", "Out 1+2", "Split"},
        .transpose_modes = { "Pitch Shift", "Scale" },
        .scales = { "Ionian", "Dorian", "Phrygian", "Lydian", "Mixolydian", "Aeolian", "Locrian" },
        .intervals = {
            "Oct Dn", "6th Dn", "5th Dn", "4th Dn", "3rd Dn",
            "3rd Up", "4th Up", "5th Up", "6th Up", "Oct Up"
        },
        .no_yes = { "No", "Yes" },
        .usb_receive_send = { "No USB", "Send USB"}
    }
};

const Message *message = &_message;
