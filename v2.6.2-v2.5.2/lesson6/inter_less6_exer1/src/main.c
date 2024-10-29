/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
/* STEP 3.1 - Include the header file of the Zephyr ADC API */


/* STEP 3.2 - Define a variable of type adc_dt_spec for each channel */


LOG_MODULE_REGISTER(Lesson6_Exercise1, LOG_LEVEL_DBG);

int main(void)
{
	int err;
	uint32_t count = 0;

	/* STEP 4.1 - Define a variable of type adc_sequence and a buffer of type uint16_t */


	/* STEP 3.3 - validate that the ADC peripheral (SAADC) is ready */

	/* STEP 3.4 - Setup the ADC channel */

	/* STEP 4.2 - Initialize the ADC sequence */


	while (1) {
		int val_mv;

		/* STEP 5 - Read a sample from the ADC */


		val_mv = (int)buf;
		LOG_INF("ADC reading[%u]: %s, channel %d: Raw: %d", count++, adc_channel.dev->name,
			adc_channel.channel_id, val_mv);

		/* STEP 6 - Convert raw value to mV*/


		k_sleep(K_MSEC(1000));
	}
	return 0;
}
