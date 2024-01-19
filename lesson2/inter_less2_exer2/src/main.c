/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

/* STEP 1.2  - Include the header file for core dump */


LOG_MODULE_REGISTER(Lesson2_Exercise2, LOG_LEVEL_INF);

/* STEP 2.1 - Define crash_function to cause a fault error */


void button_handler(uint32_t button_state, uint32_t has_changed)
{
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
            case DK_BTN1_MSK:
                LOG_INF("Button 1 pressed");
				/* STEP 2.2 - Call crash_function() when button 1 is pressed */

                break;
			default:
				break;
		}
    }
}

int main(void)
{
    if (dk_buttons_init(button_handler)) {
        LOG_ERR("Failed to initialize the buttons library");
    }

	LOG_INF("Press button 1 to get a fault error");

	return 0;
}