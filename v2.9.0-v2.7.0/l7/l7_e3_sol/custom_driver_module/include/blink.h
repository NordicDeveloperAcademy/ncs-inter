/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_DRIVERS_BLINK_H_
#define APP_DRIVERS_BLINK_H_

#include <zephyr/device.h>
#include <zephyr/toolchain.h>


/* STEP 2.1 Define the API structure */
__subsystem struct blink_driver_api {
	/**
	 * @brief Configure the LED blink period.
	 *
	 * @param dev Blink device instance.
	 * @param period_ms Period of the LED blink in milliseconds, 0 to
	 * disable blinking.
	 *
	 * @retval 0 if successful.
	 * @retval -EINVAL if @p period_ms can not be set.
	 * @retval -errno Other negative errno code on failure.
	 */
	int (*set_period_ms)(const struct device *dev, unsigned int period_ms);
};

 /* STEP 2.3 Provide the user space wrapper with the prefix syscall before the API function declaration */
__syscall int blink_set_period_ms(const struct device *dev,
				  unsigned int period_ms);

 /* STEP 2.2 Implement a public API function for the driver class */
static inline int z_impl_blink_set_period_ms(const struct device *dev,
					     unsigned int period_ms)
{
	__ASSERT_NO_MSG(DEVICE_API_IS(blink, dev));

	return DEVICE_API_GET(blink, dev)->set_period_ms(dev, period_ms);
}


/* STEP 2.4 Add blink_off api function */
static inline int blink_off(const struct device *dev)
{
	return blink_set_period_ms(dev, 0);
}

/* STEP 2.5 Add the syscall header at the end of the header file */
#include <syscalls/blink.h>
/** @} */

/** @} */

#endif /* APP_DRIVERS_BLINK_H_ */
