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
struct data_item_t {
	void *fifo_reserved;
	uint8_t data[MAX_DATA_SIZE];
	uint16_t len;
};

/* STEP 3 - Define the FIFO */
K_FIFO_DEFINE(my_fifo);

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
	while (1) {
		int bytes_written;
		/* Generate a random number between MIN_DATA_ITEMS & MAX_DATA_ITEMS to represent the
		number of data items to send every time the producer thread is scheduled */
		uint32_t data_number =
			MIN_DATA_ITEMS + sys_rand32_get() % (MAX_DATA_ITEMS - MIN_DATA_ITEMS + 1);
		for (int i = 0; i <= data_number; i++) {
			/* Create a data item to send */
			struct data_item_t *buf = k_malloc(sizeof(struct data_item_t));
			if (buf == NULL) {
				/* Unable to locate memory from the heap */
				LOG_ERR("Enable to allocate memory");
				return;
			}
			bytes_written = snprintf(buf->data, MAX_DATA_SIZE, "Data Seq. %u:\t%u",
						 dataitem_count, sys_rand32_get());
			buf->len = bytes_written;
			dataitem_count++;
			k_fifo_put(&my_fifo, buf);
		}
		LOG_INF("Producer: Data Items Generated: %u", data_number);
		k_msleep(PRODUCER_SLEEP_TIME_MS);
	}
}

static void consumer_func(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);
	/*STEP 6 - Complete the consumer thread functionality */
	while (1) {
		struct data_item_t *rec_item;
		rec_item = k_fifo_get(&my_fifo, K_FOREVER);
		LOG_INF("Consumer: %s\tSize: %u", rec_item->data, rec_item->len);
		k_free(rec_item);
	}
}

K_THREAD_DEFINE(producer, STACKSIZE, producer_func, NULL, NULL, NULL, PRODUCER_THREAD_PRIORITY, 0,
		0);
K_THREAD_DEFINE(consumer, STACKSIZE, consumer_func, NULL, NULL, NULL, CONSUMER_THREAD_PRIORITY, 0,
		0);
