/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);

int main(void)
{
	
	LOG_INF("Starting Exercise 1!");

	/* STEP 4 - Add some logic to the application */
	int8_t test_var = 124;
	for (int i = 0; i < 10; i++)
	{
		test_var = test_var + 1;
		LOG_INF("test_var = %d",test_var);
	}

	return 0;
}
