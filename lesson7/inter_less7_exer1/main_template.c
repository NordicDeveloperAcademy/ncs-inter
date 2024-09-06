/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Lesson7_Exercise1, LOG_LEVEL_INF);

/* STEP 15 - Get the device structure from the node label */

int main(void)
{
	int err;

	LOG_INF("Lesson 7 - Exercise 1 started");

	/* STEP 16.1 - Define variables to store the temperature, pressure and humidity */

	err = device_is_ready(dev);
	if (!err) {
		LOG_INF("Error: SPI device is not ready, err: %d", err);
		return 0;
	}

	while (1) {
		/* STEP 16.2 - Continuously read out sensor data using the sensor API calls */

		k_sleep(K_MSEC(1000));
	}
	
	return 0;
}