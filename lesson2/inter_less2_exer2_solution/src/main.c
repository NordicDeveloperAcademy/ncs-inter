/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

/* STEP 1.2  - Include the header file for core dump */
#include <zephyr/debug/coredump.h>

LOG_MODULE_REGISTER(Lesson2_Exercise2, LOG_LEVEL_INF);

/* STEP 2.1 - Define crash_function to cause a fault error */
void crash_function(uint32_t *addr)
{
	LOG_INF("Button pressed at %" PRIu32, k_cycle_get_32());
	LOG_INF("Coredump: %s", CONFIG_BOARD);

	#if !defined(CONFIG_CPU_CORTEX_M)
	/* For null pointer reference */
	*addr = 0;
	#else
		ARG_UNUSED(addr);
		/* Dereferencing null-pointer in TrustZone-enabled
	 	* builds may crash the system, so use, instead an
	 	* undefined instruction to trigger a CPU fault.
	 	*/
	__asm__ volatile("udf #0" : : : );
#endif
}

void button_handler(uint32_t button_state, uint32_t has_changed)
{
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
            case DK_BTN1_MSK:
                LOG_INF("Button 1 pressed");
				/* STEP 2.2 - Call crash_function() when button 1 is pressed */
                crash_function(0);
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