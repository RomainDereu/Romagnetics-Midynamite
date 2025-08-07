/*
 * test_utils.c
 * Author: Romain Dereu
 */

#include "../10_Source_code/Core/Inc/settings.h"
#include "../10_Source_code/Core/Inc/saving.h"
#include "test_utils.h"

void initialize_memory(){

    save_struct defaults = make_default_settings();
    store_settings(&defaults);
    load_settings();

}