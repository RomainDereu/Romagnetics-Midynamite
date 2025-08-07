
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "unity.h"
#include "test_utils.h"
#include "../10_Source_code/Core/Inc/midi_modify.h"
#include "../10_Source_code/Core/Inc/settings.h"
#include "../10_Source_code/Core/Inc/saving.h"


// Declare externs so test can access the single definitions in globals.c
extern midi_modify_data_struct midi_modify_data;
extern midi_transpose_data_struct midi_transpose_data;
extern settings_data_struct settings_data;
extern midi_modify_circular_buffer midi_modify_buff;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

static uint8_t clamp0_127(int16_t v) {
    if (v < 0) return 0;
    if (v > 127) return 127;
    return (uint8_t)v;
}

static void get_uart_tx(UART_HandleTypeDef *huart, 
                        uint8_t *out_buf, size_t *out_len) {
    size_t len;
    const uint8_t *data = mock_uart_get_tx_data(huart, &len);
    if (out_len) *out_len = len;
    if (out_buf && data && len) {
        memcpy(out_buf, data, len);
    }
}

static void setup_midi_modify(const midi_modify_data_struct *src)
{
    midi_modify_data = *src;  // copies all fields at once
}

void setUp(void) {
    srand(1234);

    initialize_memory();

    srand((unsigned)time(NULL));

    mock_uart_reset(&huart1);
    mock_uart_reset(&huart2);
}

void tearDown(void) {
    // nothing
}


void test_velocity_addition(void) {
    uint8_t base_velocity = rand() % 128;
    int8_t velocity_change = 50-(rand() % 101);
    uint8_t expected_velocity = clamp0_127(base_velocity + velocity_change);


    midi_note msg = {0};
    msg.status = 0x90;
    msg.note = 60;
    msg.velocity = base_velocity;
    

    midi_modify_data_struct params = {
        .change_or_split        = rand() & 1,
        .velocity_type          = MIDI_MODIFY_CHANGED_VEL,
        .send_to_midi_out       = MIDI_OUT_1,
        .send_to_midi_channel_1 = rand() & 15,
        .send_to_midi_channel_2 = rand() & 15,
        .split_note             = rand() & 127,
        .split_midi_channel_1   = rand() & 15,
        .split_midi_channel_2   = rand() & 15,
        .velocity_plus_minus    = velocity_change,
        .velocity_absolute      = rand() & 127,
        .currently_sending      = 1
            };

    setup_midi_modify(&params);

    settings_data.midi_thru = 0; 

    pipeline_start(&msg);

    uint8_t sent[8] = {0};
    size_t sent_len = 0;
    get_uart_tx(&huart1, sent, &sent_len);

    TEST_ASSERT_EQUAL_HEX8(expected_velocity, sent[2]);
}

void test_velocity_absolute(void) {
    uint8_t fixed_velocity = rand() % 128;

    midi_note msg = {0};
    msg.status = 0x90;
    msg.note = 60;
    msg.velocity = fixed_velocity;

midi_modify_data_struct params = {
    .change_or_split        = rand() & 1,
    .velocity_type          = MIDI_MODIFY_FIXED_VEL,
    .send_to_midi_out       = MIDI_OUT_1,
    .send_to_midi_channel_1 = rand() & 15,
    .send_to_midi_channel_2 = rand() & 15,
    .split_note             = rand() & 127,
    .split_midi_channel_1   = rand() & 15,
    .split_midi_channel_2   = rand() & 15,
    .velocity_plus_minus    = rand() & 15,
    .velocity_absolute      = fixed_velocity,
    .currently_sending      = 1
};
            
    setup_midi_modify(&params);

    
    settings_data.midi_thru = 0; 

    pipeline_start(&msg);

    uint8_t sent[8] = {0};
    size_t sent_len = 0;
    get_uart_tx(&huart1, sent, &sent_len);

    TEST_ASSERT_EQUAL_HEX8(fixed_velocity, sent[2]);
    }