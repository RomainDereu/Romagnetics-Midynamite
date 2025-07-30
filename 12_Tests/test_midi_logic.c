#ifdef UNIT_TEST

#include "unity.h"
#include "midi_modify.h"

void setUp(void) {
    // optional
}
void tearDown(void) {
    // optional
}

void test_velocity_addition(void) {
    uint8_t msg[] = {0x90, 60, 64};
    midi_modify_data.velocity_plus_minus = 10;
    midi_modify_data.velocity_type = 0;

    change_velocity(msg);
    TEST_ASSERT_EQUAL_UINT8(74, msg[2]);
}


#endif
