/*
 * settings.c
 *
 *  Created on: Apr 2, 2025
 *      Author: Romain Dereu
 */

#define SETTINGS_MM_CHANNEL_CHANGE 0
#define SETTINGS_MM_CHANNEL_SPLIT 1

#define SETTINGS_MM_VELOCITY_CHANGE 0
#define SETTINGS_MM_VELOCITY_FIXED 1


#include <string.h>

#include "cmsis_os.h"
#include "saving.h"
#include "main.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"

#include "stm32f4xx_hal.h"

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;
extern midi_transpose_data_struct midi_transpose_data;
extern settings_data_struct settings_data;


save_struct creating_save(midi_tempo_data_struct * midi_tempo_data_to_save,
		                  midi_modify_data_struct * midi_modify_data_to_save,
						  midi_transpose_data_struct * midi_transpose_data_to_save,
						  settings_data_struct *settings_data_to_save){
	save_struct this_save;
	this_save.midi_tempo_data = * midi_tempo_data_to_save;
	this_save.midi_modify_data = * midi_modify_data_to_save;
	this_save.midi_transpose_data = * midi_transpose_data_to_save;
	this_save.settings_data = * settings_data_to_save;
	//Random number, just to check that the data works
	this_save.check_data_validity = DATA_VALIDITY_CHECKSUM;
	return this_save;
}






HAL_StatusTypeDef store_settings(save_struct *data){
	HAL_StatusTypeDef status;
	uint32_t error_status = 0;
	FLASH_EraseInitTypeDef flash_erase_struct = {0};

	//Regarding the data
    uint32_t* data_ptr = (uint32_t*)data;
    uint32_t data_length = sizeof(save_struct) / 4;

	HAL_FLASH_Unlock();
	//erasing the memory
	flash_erase_struct.TypeErase = FLASH_TYPEERASE_SECTORS;
	flash_erase_struct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	flash_erase_struct.Sector = FLASH_SECTOR_7;
	flash_erase_struct.NbSectors = 1; //Specify num of sectors
	HAL_FLASHEx_Erase(&flash_erase_struct, &error_status);

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
                           FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    status = HAL_FLASHEx_Erase(&flash_erase_struct, &error_status);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return status;
    }

    // Write data (must be 32-bit aligned)
    for (uint32_t i = 0; i < data_length; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_SECTOR7_ADDR + i * 4, data_ptr[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return status;
        }
    }

    // Lock Flash again
    HAL_FLASH_Lock();
    return HAL_OK;
}


save_struct read_setting_memory(void){ 
	    save_struct flash_data;
	    memcpy(&flash_data, (void*)FLASH_SECTOR7_ADDR, sizeof(flash_data));
	    return flash_data;
}

save_struct make_default_settings(void){

	save_struct emergency_save;

	//midi_tempo_data
	emergency_save.midi_tempo_data.current_tempo = 60;
	emergency_save.midi_tempo_data.currently_sending = 0;
	emergency_save.midi_tempo_data.send_channels = MIDI_OUT_1_2;

	//midi_modify_data
	emergency_save.midi_modify_data.change_or_split = MIDI_MODIFY_CHANGE;
	emergency_save.midi_modify_data.velocity_type = MIDI_MODIFY_CHANGED_VEL;


	emergency_save.midi_modify_data.send_to_midi_channel = 1;
	emergency_save.midi_modify_data.send_to_midi_out = MIDI_OUT_1;

	emergency_save.midi_modify_data.split_note = 0;
	emergency_save.midi_modify_data.split_midi_channel_1 = 1;
	emergency_save.midi_modify_data.split_midi_channel_2 = 2;

	emergency_save.midi_modify_data.velocity_plus_minus = 0;
	emergency_save.midi_modify_data.velocity_absolute = 127;

	//midi_transpose_data
	emergency_save.midi_transpose_data.transpose_type = MIDI_TRANSPOSE_SHIFT;

	emergency_save.midi_transpose_data.midi_shift_value = 12;

	emergency_save.midi_transpose_data.transpose_base_note = 0;
	emergency_save.midi_transpose_data.transpose_scale = IONIAN;
	emergency_save.midi_transpose_data.transpose_interval = THIRD_DOWN;

	emergency_save.midi_transpose_data.send_channels = MIDI_OUT_1;

	//settings_data
	emergency_save.settings_data.start_menu = MIDI_TEMPO;
	emergency_save.settings_data.brightness = 0xFF;

	//Checksum
	emergency_save.check_data_validity = DATA_VALIDITY_CHECKSUM;

	return emergency_save;
}


void load_settings(){
	  //Initiation of the memory
	  save_struct flash_save = read_setting_memory();
	  //Cheking if the save is valid
	  if (flash_save.check_data_validity == DATA_VALIDITY_CHECKSUM){
		  //Overwrite the default values
		  midi_tempo_data = flash_save.midi_tempo_data;
		  midi_modify_data = flash_save.midi_modify_data;
		  midi_transpose_data = flash_save.midi_transpose_data;
		  settings_data = flash_save.settings_data;
	  }
	  //If the save is corrupt, use default values
	  else {

		  save_struct emergency_save = make_default_settings();
		  store_settings(&emergency_save);
		  load_settings();
	  }


}
