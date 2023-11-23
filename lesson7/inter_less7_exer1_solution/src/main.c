/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "custom_bme280_driver_spi.h"
#include <stdio.h>
#include <zephyr/kernel.h>

const struct device *dev;

int main(void)
{
	printk("Main started in the app!\n");

	dev = device_get_binding("CUSTOM_BME280_DRIVER");

	__ASSERT(dev, "Failed to get device binding");

	printk("device is %p, name is %s\n", dev, dev->name);

	custom_bme280_print(dev);

	return 0;
}
