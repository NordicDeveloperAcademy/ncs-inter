/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

#include "custom_bme280_driver_spi.h"

LOG_MODULE_REGISTER(Lesson7_Exercise1, LOG_LEVEL_INF);

#define SPIOP	SPI_WORD_SET(8) | SPI_TRANSFER_MSB

const struct gpio_dt_spec ledspec = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(bme280));

int main(void)
{
	int err;

	LOG_INF("Lesson 7 - Exercise 1 started");

	err = gpio_is_ready_dt(&ledspec);
	if (!err) {
		LOG_INF("Error: GPIO device is not ready, err: %d", err);
		return 0;
	}

	err = device_is_ready(spi_dev);
	if (!err) {
		LOG_INF("Error: SPI device is not ready, err: %d", err);
		return 0;
	}

	gpio_pin_configure_dt(&ledspec, GPIO_OUTPUT_ACTIVE);
	
	custom_bme280_print(spi_dev);

	while(1){
		gpio_pin_toggle_dt(&ledspec);
		k_msleep(3000);
	}
	
	return 0;
}
