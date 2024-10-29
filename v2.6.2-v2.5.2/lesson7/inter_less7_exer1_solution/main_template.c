/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Lesson7_Exercise1, LOG_LEVEL_INF);

/* STEP 16 - Get the device structure from the node label */
const struct device * dev = DEVICE_DT_GET(DT_NODELABEL(bme280));

int main(void)
{
	int err;

	LOG_INF("Lesson 7 - Exercise 1 started");

	/* STEP 17.1 - Define variables to store the temperature, pressure and humidity */
	struct sensor_value temp_val, press_val, hum_val;

	err = device_is_ready(dev);
	if (!err) {
		LOG_INF("Error: SPI device is not ready, err: %d", err);
		return 0;
	}

	while (1) {
		/* STEP 17.2 - Continuously read out sensor data using the sensor API calls */
		err = sensor_sample_fetch(dev);
		if (err < 0) {
			LOG_ERR("Could not fetch sample (%d)", err);
			return 0;
		}

		if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_val)) {
			LOG_ERR("Could not get sample");
			return 0;
		}
		
		if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press_val)) {
			LOG_ERR("Could not get sample");
			return 0;
		}
	
		if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum_val)) {
			LOG_ERR("Could not get sample");
			return 0;
		}

		LOG_INF("Compensated temperature value: %d", temp_val.val1);
		LOG_INF("Compensated pressure value: %d", press_val.val1);
		LOG_INF("Compensated humidity value: %d", hum_val.val1);
		
		k_sleep(K_MSEC(1000));
	}
	
	return 0;
}