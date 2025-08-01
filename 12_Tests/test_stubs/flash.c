// flash.c — test override for saving.c (in-memory, no real flash)
#include "../10_Source_code/Core/Inc/main.h"
#include "../10_Source_code/Core/Inc/saving.h"
#include <string.h>

// ---------- Simulated flash storage ----------
static save_struct simulated_flash = {0};

// ---------- Stub out flash/HAL machinery so saving.c can call it safely ----------
#define FLASH_SECTOR7_ADDR ((uint32_t)0x08060000) // unused in test override
#define FLASH_TYPEERASE_SECTORS 0
#define FLASH_VOLTAGE_RANGE_3 0
#define FLASH_SECTOR_7 0

#define __HAL_FLASH_CLEAR_FLAG(x) (void)(x)
static inline void HAL_FLASH_Unlock(void) {}
static inline void HAL_FLASH_Lock(void) {}
static inline int HAL_FLASHEx_Erase(void *eraseStruct, uint32_t *err) {
    (void)eraseStruct; (void)err;
    return 0; // pretend success
}
static inline int HAL_FLASH_Program(uint32_t typeprog, uint32_t address, uint32_t data) {
    (void)typeprog; (void)address; (void)data;
    return 0; // pretend success
}

// ---------- Override production storage/read functions ----------
save_struct read_setting_memory(void) {
    return simulated_flash;
}

HAL_StatusTypeDef store_settings(save_struct *data) {
    if (!data) return HAL_ERROR;
    simulated_flash = *data;
    return HAL_OK;
}
