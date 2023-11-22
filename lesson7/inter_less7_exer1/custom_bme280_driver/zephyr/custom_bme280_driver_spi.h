/*
 * Copyright (c) 2019 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __HELLO_WORLD_DRIVER_H__
#define __HELLO_WORLD_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/device.h>

/*
 * This 'Hello World' driver has a 'print' syscall that prints the
 * famous 'Hello World!' string.
 *
 * The string is formatted with some internal driver data to
 * demonstrate that drivers are initialized during the boot process.
 *
 * The driver exists to demonstrate (and test) custom drivers that are
 * maintained outside of Zephyr.
 */

typedef	void (*custom_bme280_api_print_t)(const struct device * dev);
typedef	int  (*custom_bme280_api_open_t)(const struct device * dev);
typedef int  (*custom_bme280_api_read_t)(const struct device * dev, uint32_t * val);
typedef int  (*custom_bme280_api_write_t)(const struct device * dev, uint32_t * val);
typedef int  (*custom_bme280_api_close_t)(const struct device * dev);


struct custom_bme280_driver_api {
	custom_bme280_api_print_t print;
	custom_bme280_api_open_t open;
	custom_bme280_api_read_t read;
	custom_bme280_api_write_t write;
	custom_bme280_api_close_t close;
};


__syscall     void        custom_bme280_print(const struct device * dev);
static inline void z_impl_custom_bme280_print(const struct device * dev)
{
	const struct custom_bme280_driver_api *api = dev->api;

	__ASSERT(api->print, "Callback pointer should not be NULL");

	api->print(dev);
}

__syscall	int  custom_bme280_open(const struct device * dev);
static inline int z_impl_custom_bme280_open(const struct device * dev)
{
	const struct custom_bme280_driver_api *api = dev->api;

	__ASSERT(api->open, "Callback pointer should not be NULL");

	return api->open(dev);
}

__syscall   int  custom_bme280_read(const struct device * dev, uint32_t * val);
static inline int z_impl_custom_bme280_read(const struct device * dev, uint32_t * val)
{
	const struct custom_bme280_driver_api *api = dev->api;

	__ASSERT(api->read, "Callback pointer should not be NULL");

	return api->read(dev, val);
}

__syscall   int  custom_bme280_write(const struct device * dev, uint32_t * val);
static inline int z_impl_custom_bme280_write(const struct device * dev, uint32_t * val)
{
	const struct custom_bme280_driver_api *api = dev->api;

	__ASSERT(api->write, "Callback pointer should not be NULL");

	return api->write(dev, val);
}

__syscall   int  custom_bme280_close(const struct device * dev);
static inline int z_impl_custom_bme280_close(const struct device * dev)
{
	const struct custom_bme280_driver_api *api = dev->api;

	__ASSERT(api->close, "Callback pointer should not be NULL");

	return api->close(dev);
}

#ifdef __cplusplus
}
#endif

#include <syscalls/custom_bme280_driver_spi.h>

#endif /* __HELLO_WORLD_DRIVER_H__ */
