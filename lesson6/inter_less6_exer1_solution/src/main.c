/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* STEP 1.3 - Include the relevant headers for pwm*/
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(Lesson6_Exercise1, LOG_LEVEL_INF);

/* STEP 2 - Define the desired PWM period and pulse */
#define PWM_PERIOD_NS   20000000
#define PWM_DUTY_CYCLE  1400000

 /* STEP 6.1 - Define a minimum and maximum period */
#define MIN_PERIOD   PWM_SEC(1U) / 128U
#define MAX_PERIOD   PWM_SEC(1U)

/* STEP 3.1 - Get the node identifier for [] through its alias */
#define PWM_LED0     DT_ALIAS(pwm_led0)

/* STEP 3.2 - Initialize and populate struct pwm_dt_spec */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(PWM_LED0);

int main(void)
{
    int err;

    /* STEP 6.2 Declare variables */
    uint32_t max_period = MAX_PERIOD;
    uint32_t period;
    uint8_t dir = 0U;

    /* STEP 3.3 - Check if the device is ready */
    if (!pwm_is_ready_dt(&pwm_led0)) {
        LOG_ERR("Error: PWM device %s is not ready", pwm_led0.dev->name);
        return 0;
	}
    
    /* STEP 4 - Control the LED with the control signal generated from the PWM */
    err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, PWM_DUTY_CYCLE);
    if (err) {
        LOG_ERR("Error in pwm_set_dt(), err: %d", err);
    }
    
    /* STEP 6.3 - */
    while (pwm_set_dt(&pwm_led0, max_period, max_period / 2U)) {
	    max_period /= 2U;
	    if (max_period < (4U * MIN_PERIOD)) {
		    LOG_ERR("Error: PWM device does not support a period at least %lu\n", 4U * MIN_PERIOD);
		    return 0;
	    }
    }
    LOG_INF("Done calibrating; maximum/minimum periods %u/%lu nsec\n", max_period, MIN_PERIOD);

    /* STEP 6.4 - Set period = max_period */
    while(1) 
    {
        /* STEP 6.5 - */
        err = pwm_set_dt(&pwm_led0, period, period / 2U);
		if (err) {
			LOG_ERR("Error %d: failed to set pulse width\n", ret);
			return 0;
		}

        /* STEP 6.6 - */
        period = dir ? (period * 2U) : (period / 2U);
        if (period > max_period) {
	        period = max_period / 2U;
	        dir = 0U;
        } else if (period < MIN_PERIOD) {
	        period = MIN_PERIOD * 2U;
	        dir = 1U;
        }

    }
    return 0;
}