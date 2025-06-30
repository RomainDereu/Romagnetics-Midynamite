/*
 * text.c
 *
 *  Created on: Jun 30, 2025
 *      Author: Romain Dereu
 */


#include "text.h"

static const Message message_data = {
    .send_midi_tempo = "Send Midi Tempo              ",
    .target =  "Target:"
};

const Message *message = &message_data;
