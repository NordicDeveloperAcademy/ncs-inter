/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* STEP 5.4 - Include header for usb */

/* 1000 msec = 1 sec */

#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* STEP 7.2 - Create context for USB stack */
/* STEP 7.3 - Create VBUS callback function */

int main(void)
{
    int ret;

    /* STEP 7.4 - Initialize usb device */
    /* STEP 7.5 - Enable usb device in case vbus not detected */

    if (!device_is_ready(led.port)) {
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    while (1) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            return 0;
        }
        k_msleep(SLEEP_TIME_MS);
    }
  return 1;
}
