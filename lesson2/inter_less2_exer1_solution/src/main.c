/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
/* STEP 7.2.1*/
#include <dk_buttons_and_leds.h>


/* STEP 7.2.2*/
#define USER_BUTTON 	DK_BTN1_MSK
static void button_changed(uint32_t button_state, uint32_t has_changed)
{
    if (has_changed & USER_BUTTON) {
        dk_set_led_on(DK_LED1);
    }
}
 
static int init_button(void)
{
    int err;
 
    err = dk_buttons_init(button_changed);
    if (err) {
        printk("Cannot init buttons (err: %d)\n", err);
    }
 
    return err;
}


LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);
int8_t test_var = 124;
int main(void)
{
	/*STEP 7.2.3*/
	int err;
 
        err = dk_leds_init();
    if (err) {
        printk("LEDs init failed (err %d)\n", err);
        return 0;
    }
 
    err = init_button();
    if (err) {
        printk("Button init failed (err %d)\n", err);
        return 0;
    }

	LOG_INF("Starting Exercise 1!");

	/* STEP 4 - Add some logic to the application */

	for (int i = 0; i < 10; i++) {
		test_var = test_var + 1;
		LOG_INF("test_var = %d", test_var);
	}

	return 0;
}
