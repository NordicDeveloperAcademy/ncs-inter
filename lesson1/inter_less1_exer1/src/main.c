/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

/* The devicetree node identifier for the "led0"  and "led1" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

/* 2200 msec = 2.2 sec */
#define PRODUCER_SLEEP_TIME_MS   2200

LOG_MODULE_REGISTER(Less1_Exer1,LOG_LEVEL_DBG);
/* Stack size for both the producer and consumer threads */
#define STACKSIZE 2048
#define PRODUCER_THREAD_PRIORITY 6
#define CONSUMER_THREAD_PRIORITY 7

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

/* STEP 2.3 - Create the expiry function for the timer */


/* STEP 2.1 - Define the timer */


/* STEP 3.1 - Define the data type of the message */


/* STEP 3.2 - Define the message queue */


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
    /* STEP 2.2 - Start the timer  */


	return 0;
}


void producer_func(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

    while (1) {
        static SensorReading acc_val ={100,100,100};
        int ret;
        /* STEP 3.3 - Write messages to the message queue */
 
 
        acc_val.x_reading += 1;
        acc_val.y_reading += 1;
        acc_val.z_reading += 1;
        k_msleep(PRODUCER_SLEEP_TIME_MS);
    }
}

void consumer_func(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

    while (1) {
        SensorReading temp;
        int ret;
        /* STEP 3.4 - Read messages from the message queue */

        LOG_INF("Values got from the queue: %d.%d.%d\r\n",temp.x_reading,temp.y_reading,temp.z_reading);
    }
}

K_THREAD_DEFINE(producer, STACKSIZE, producer_func, NULL, NULL, NULL,
        PRODUCER_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(consumer, STACKSIZE, consumer_func, NULL, NULL, NULL,
        CONSUMER_THREAD_PRIORITY, 0, 0);
