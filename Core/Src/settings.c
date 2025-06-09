/*
 * settings.c
 *
 *  Created on: Apr 2, 2025
 *      Author: Astaa
 */


#include "cmsis_os.h"
#include "settings.h"
#include "main.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"

#include "stm32f4xx_hal.h"

extern struct midi_tempo_data_struct midi_tempo_data;



void screen_update_settings(){
char save_settings_message[30] = "Save Settings                 ";
screen_driver_SetCursor(0, 56);
screen_driver_WriteString(save_settings_message, Font_6x8, White);
screen_driver_UpdateScreen();
}


struct save_struct creating_save(struct midi_tempo_data_struct * midi_tempo_data_to_save){
	struct save_struct this_save;
	this_save.midi_tempo_data = * midi_tempo_data_to_save;
	//Random number, just to check that the data works
	this_save.check_data_validity = 42817;
	return this_save;
}


void settings_saved(){
	uint8_t Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
	  if(Btn1State == 0)
		 {
			 //Debouncing
			 osDelay(10);
			 Btn1State = HAL_GPIO_ReadPin(GPIOB, Btn1_Pin);
			  if(Btn1State == 0)
				 {
				 //Saving the current configuration to the memory
				 struct save_struct memory_to_be_saved = creating_save(&midi_tempo_data);
				 store_settings(&memory_to_be_saved);

				 char saved_print[6] = "Saved!";
				 char saved__clear_print[6] = "      ";
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



HAL_StatusTypeDef store_settings(struct save_struct *data){
	HAL_StatusTypeDef status;
	uint32_t error_status = 0;
	FLASH_EraseInitTypeDef flash_erase_struct = {0};

	//Regarding the data
    uint32_t* data_ptr = (uint32_t*)data;
    uint32_t data_length = sizeof(struct save_struct) / 4;

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


struct save_struct read_settings(void){
	    struct save_struct flash_data;
	    memcpy(&flash_data, (void*)FLASH_SECTOR7_ADDR, sizeof(flash_data));
	    return flash_data;
}
