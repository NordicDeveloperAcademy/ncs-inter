/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
/* The devicetree node identifier for the "led0"  and "led1" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

/* 2200 msec = 2.2 sec */
#define PRODUCER_SLEEP_TIME_MS 2200

LOG_MODULE_REGISTER(Less1_Exer2, LOG_LEVEL_DBG);
/* Stack size for both the producer and consumer threads */
#define STACKSIZE		 2048
#define PRODUCER_THREAD_PRIORITY 6
#define CONSUMER_THREAD_PRIORITY 7
#define MAX_DATA_SIZE		 32
#define MIN_DATA_ITEMS		 4
#define MAX_DATA_ITEMS		 14
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

/* STEP 4 - Define the data type of the FIFO items*/

/* STEP 3 - Define the FIFO */

static void timer0_handler(struct k_timer *dummy)
{

	/*Interrupt Context - System Timer ISR */
	static bool flip = true;
	if (flip) {
		gpio_pin_toggle_dt(&led0);
	} else {
		gpio_pin_toggle_dt(&led1);
	}

	flip = !flip;
}

static K_TIMER_DEFINE(timer0, timer0_handler, NULL);

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led0)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	/* start periodic timer that expires once every 0.5 second  */
	k_timer_start(&timer0, K_MSEC(500), K_MSEC(500));

	return 0;
}

static void producer_func(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);
	static uint32_t dataitem_count = 0;
	/*STEP 5  - Complete the producer thread functionality  */
}

static void consumer_func(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);
	/*STEP 6 - Complete the consumer thread functionality */
}

K_THREAD_DEFINE(producer, STACKSIZE, producer_func, NULL, NULL, NULL, PRODUCER_THREAD_PRIORITY, 0,
		0);
K_THREAD_DEFINE(consumer, STACKSIZE, consumer_func, NULL, NULL, NULL, CONSUMER_THREAD_PRIORITY, 0,
		0);
