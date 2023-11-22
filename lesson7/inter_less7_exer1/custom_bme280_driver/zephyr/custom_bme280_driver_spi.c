/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "custom_bme280_driver_spi.h"
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <zephyr/syscall_handler.h>

/**
 * This is a minimal example of an out-of-tree driver
 * implementation. See the header file of the same name for details.
 */

static struct hello_world_dev_data {
	uint32_t foo;
} data;

static int init(const struct device *dev)
{
	data.foo = 5;

	return 0;
}

static void bme280_sample_print(const struct device *dev)
{
	printk("Print all characteristics of BME280 sensor\n");

	// __ASSERT();
}

static int bme280_sample_open(const struct device *dev)
{
	printk("Print all characteristics of BME280 sensor\n");

	return 0;
}

static int bme280_sample_close(const struct device *dev)
{
	printk("Print all characteristics of BME280 sensor\n");

	return 0;
}

static int bme280_sample_read(const struct device *dev, uint32_t * val)
{
	printk("Print all characteristics of BME280 sensor\n");

	return 0;
}

static int bme280_sample_write(const struct device *dev, uint32_t * val)
{
	printk("Print all characteristics of BME280 sensor\n");

	return 0;
}

static const struct custom_bme280_driver_api custom_bme280_api_funcs = {
	.print  = bme280_sample_print,
	.open   = bme280_sample_open,
	.write  = bme280_sample_write,
	.read   = bme280_sample_read,
	.close  = bme280_sample_close,
};

DEVICE_DEFINE(inter_less7_exer1, "CUSTOM_BME280_DRIVER",
		    init, NULL, &data, NULL,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &custom_bme280_api_funcs);
