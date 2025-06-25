/*
 * settings.c
 *
 *  Created on: Apr 2, 2025
 *      Author: Astaa
 */

#include <string.h>

#include "cmsis_os.h"
#include "settings.h"
#include "main.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"

#include "stm32f4xx_hal.h"

extern midi_tempo_data_struct midi_tempo_data;
extern midi_modify_data_struct midi_modify_data;

void screen_update_settings(){
char save_settings_message[30] = "Save Settings                 ";
screen_driver_SetCursor(0, 56);
screen_driver_WriteString(save_settings_message, Font_6x8, White);
screen_driver_UpdateScreen();
}

void list_of_UART_to_send_to(uint8_t send_channels,
                           	 UART_HandleTypeDef **UART_list){
	extern UART_HandleTypeDef huart1;
	extern UART_HandleTypeDef huart2;

	if (send_channels == MIDI_OUT_1){
		UART_list[0] = &huart1;
		UART_list[1] = NULL;
	}

	else if (send_channels == MIDI_OUT_2){
		UART_list[0] = &huart2;
		UART_list[1] = NULL;
	}

	else if (send_channels == MIDI_OUT_1_2){
		UART_list[0] = &huart1;
		UART_list[1] = &huart2;
	}

	else{
		UART_list[0] = NULL;
		UART_list[1] = NULL;
	}

}


save_struct creating_save(midi_tempo_data_struct * midi_tempo_data_to_save,
		                  midi_modify_data_struct * midi_modify_data_to_save){
	save_struct this_save;
	this_save.midi_tempo_data = * midi_tempo_data_to_save;
	this_save.midi_modify_data = * midi_modify_data_to_save;
	//Random number, just to check that the data works
	this_save.check_data_validity = DATA_VALIDITY_CHECKSUM;
	return this_save;
}


void saving_settings_ui(){
	uint8_t Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
	  if(Btn1State == 0)
		 {
			 //Debouncing
			 osDelay(10);
			 Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
			  if(Btn1State == 0)
				 {
				 //Saving the current configuration to the memory
				 char saving_print[6] = "Saving";
				 char saved_print[6] = "Saved!";
				 char saved__clear_print[6] = "      ";

				 screen_driver_SetCursor(90, 56);
				 screen_driver_WriteString(saving_print, Font_6x8, White);
				 screen_driver_UpdateScreen();

				 save_struct memory_to_be_saved = creating_save(&midi_tempo_data, &midi_modify_data);
				 store_settings(&memory_to_be_saved);


				 screen_driver_SetCursor(90, 56);
				 screen_driver_WriteString(saved_print, Font_6x8, White);
				 screen_driver_UpdateScreen();
				 osDelay(1000);
				 screen_driver_SetCursor(90, 56);
				 screen_driver_WriteString(saved__clear_print, Font_6x8, White);
				 screen_driver_UpdateScreen();
				  }
	  }

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
	  midi_tempo_data.current_tempo = 60;
	  midi_tempo_data.currently_sending = 0;
	  midi_tempo_data.send_channels = MIDI_OUT_1_2;

	  midi_modify_data.sent_to_midi_channel = 1;
	  midi_modify_data.send_2 = 0;
	  midi_modify_data.send_channels = 0;
}


void load_settings(){
	  //Initiation of the memory
	  save_struct flash_save = read_setting_memory();
	  //Cheking if the save is valid
	  if (flash_save.check_data_validity == DATA_VALIDITY_CHECKSUM){
		  //Overwrite the default values
		  midi_tempo_data = flash_save.midi_tempo_data;
		  midi_modify_data = flash_save.midi_modify_data;
	  }
	  //If the save is corrupt, use default values
	  else {

		  save_struct emergency_save = make_default_settings();
		  store_settings(&emergency_save);
	  }


}
