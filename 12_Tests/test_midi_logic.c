#include "../10_Source_code/Core/Inc/midi_modify.h"
#include "unity.h"
#include <stdint.h>

// Definitions (must match typedefs in main.h). 
midi_modify_data_struct midi_modify_data = {0};
midi_transpose_data_struct midi_transpose_data = {0};
settings_data_struct settings_data = {0};
midi_modify_circular_buffer midi_modify_buff = {0};

// Dummy UART handles if referenced globally
UART_HandleTypeDef huart1 = {0};
UART_HandleTypeDef huart2 = {0};

void setUp(void) {
    // reset globals if needed
    midi_modify_data.velocity_plus_minus = 0;
    midi_modify_data.velocity_type = 0;
}

void tearDown(void) {
    // nothing
}

void test_velocity_addition(void) {
    uint8_t msg[3] = {0x90, 60, 100};
    midi_modify_data.velocity_plus_minus = 10;
    TEST_ASSERT_EQUAL_HEX8(10, midi_modify_data.velocity_plus_minus); 
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_velocity_addition);
    return UNITY_END();
}