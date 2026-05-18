/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "cloud_connection.h"

#define CONNECTION_THREAD_STACK_SIZE 2048

/* Define, and automatically start the cloud connection thread. See cloud_connection.c */
K_THREAD_DEFINE(con_thread, CONNECTION_THREAD_STACK_SIZE, cloud_connection_thread_fn,
		NULL, NULL, NULL, 0, 0, 0);

int main(void)
{

  k_sleep(K_SECONDS(10));
	printf("Hello World! %s\n", CONFIG_BOARD);
	return 0;
}
