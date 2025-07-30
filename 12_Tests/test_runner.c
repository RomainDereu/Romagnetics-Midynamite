#ifdef UNIT_TEST

#include "unity.h"
#include "test_runner.h"
#include <stdio.h>

extern void test_velocity_addition(void);

void unity_run_all_tests(void)
{
    printf("[TEST RUN STARTED]\n");

    UNITY_BEGIN();
    RUN_TEST(test_velocity_addition);
    UNITY_END();

    printf("[TEST RUN COMPLETE]\n");
}

#endif
