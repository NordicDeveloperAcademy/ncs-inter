/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main()
{
	int ret;
	bool led_is_on = true;

	if (!gpio_is_ready_dt(&led)) {
		printk("Error: device not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: LED configuration failed\n");
		return ret;
	}
	printk("This application is running on the Application Core (CPUAPP)\n");
	printk("Hello from DevAcademy Intermediate, Lesson 8, Exercise 2\n");
	while (1) {

		led_is_on = !led_is_on;
		gpio_pin_toggle_dt(&led);
		printk("LED status: %s\n", led_is_on ? "ON" : "OFF");

		k_sleep(K_MSEC(5000));
	}

	return 0;
}
