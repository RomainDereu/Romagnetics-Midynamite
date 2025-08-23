/*
 * midi_usb.c
 *
 *  Created on: Jul 18, 2025
 *      Author: Romain Dereu
 */


#include "usbd_midi.h"
#include "memory_main.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern settings_data_struct settings_data;

void send_usb_midi_message(uint8_t *midi_message, uint8_t length) {
    if (USBD_MIDI_GetState(&hUsbDeviceFS) != MIDI_IDLE) return;

    if(settings_data.send_to_usb == USB_MIDI_SEND){

		uint8_t cin;
		uint8_t status = midi_message[0];

		switch (status) {
			case 0xF8: // Timing Clock
			case 0xFA: // Start
			case 0xFB: // Continue
			case 0xFC: // Stop
			case 0xFE: // Active Sensing
			case 0xFF: // Reset
				cin = 0x0F; // Single-byte message
				break;

			default:
				switch (status & 0xF0) {
					case 0x80: case 0x90: case 0xA0:
					case 0xB0: case 0xE0:
						cin = 0x08; break;  // 3-byte messages
					case 0xC0: case 0xD0:
						cin = 0x0C; break;  // 2-byte messages
					case 0xF0:
						cin = 0x05; break;  // Start of SysEx
					default:
						cin = 0x0F; break;  // Default to single-byte
				}
				break;
		}


		uint8_t packetsBuffer[4] = {
			(0x00 << 4) | cin,  // Cable 0
			midi_message[0],
			(length > 1) ? midi_message[1] : 0,
			(length > 2) ? midi_message[2] : 0
		};

		USBD_MIDI_SendPackets(&hUsbDeviceFS, packetsBuffer, 4);
    }
}
