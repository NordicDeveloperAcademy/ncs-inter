/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* Step 3 - Include header for nrfx SAADC driver */
#include <nrfx_saadc.h>

/* Step 4.1 - Declare the struct to hold the configuration for the SAADC channel used to sample the battery voltage */
static nrfx_saadc_channel_t channel = NRFX_SAADC_DEFAULT_CHANNEL_SE(NRF_SAADC_INPUT_VDD, 0);

/* Step 4.2 - Declare the buffer to hold the SAAD sample value */
static nrf_saadc_value_t sample;

/* Step 5.1 - Define the battery sample interval */
#define BATTERY_SAMPLE_INTERVAL_MS 2000

/* Step 5.3 - Add forward declaration of timer callback handler */
static void battery_sample_timer_handler(struct k_timer *timer);

/* Step 5.2 - Define the battery sample timer instance */
K_TIMER_DEFINE(battery_sample_timer, battery_sample_timer_handler, NULL);

/* Step 8.1 - Implement timer callback handler function */
void battery_sample_timer_handler(struct k_timer *timer)
{

        /* Step 8.2 - Trigger the sampling */
        nrfx_err_t err = nrfx_saadc_mode_trigger();
        if (err != NRFX_SUCCESS) {
                printk("nrfx_saadc_mode_trigger error: %08x", err);
                return;
        }

        /* Step 8.3 - Calculate and print voltage */
        int battery_voltage = ((600*6) * sample) / ((1<<12));

        printk("SAADC sample: %d\n", sample);
        printk("Battery Voltage: %d mV\n", battery_voltage);

}

static void configure_saadc(void)
{
        /* Step 6.1 - Connect ADC interrupt to nrfx interrupt handler */
        IRQ_CONNECT(DT_IRQN(DT_NODELABEL(adc)),
		    DT_IRQ(DT_NODELABEL(adc), priority),
		    nrfx_isr, nrfx_saadc_irq_handler, 0);
        
        /* Step 6.2 - Connect ADC interrupt to nrfx interrupt handler */
        nrfx_err_t err = nrfx_saadc_init(DT_IRQ(DT_NODELABEL(adc), priority));
        if (err != NRFX_SUCCESS) 
        {
                printk("nrfx_saadc_mode_trigger error: %08x", err);
                return;
        }

        /* Step 6.3 - Configure the SAADC channel */
        channel.channel_config.gain = NRF_SAADC_GAIN1_6;
        err = nrfx_saadc_channels_config(&channel, 1);
        if (err != NRFX_SUCCESS) 
        {
		printk("nrfx_saadc_channels_config error: %08x", err);
	        return;
	}

        /* Step 6.4 - Configure nrfx_SAADC driver in simple and blocking mode */
        err = nrfx_saadc_simple_mode_set(BIT(0),
                                         NRF_SAADC_RESOLUTION_12BIT,
                                         NRF_SAADC_OVERSAMPLE_DISABLED,
                                         NULL);
        if (err != NRFX_SUCCESS) {
                printk("nrfx_saadc_simple_mode_set error: %08x", err);
                return;
        }
        
        /* Step 6.5 - Set buffer where sample will be stored */
        err = nrfx_saadc_buffer_set(&sample, 1);
        if (err != NRFX_SUCCESS) {
                printk("nrfx_saadc_buffer_set error: %08x", err);
                return;
        }

        /* Step 7 - Start periodic timer for battery sampling */
	k_timer_start(&battery_sample_timer, K_NO_WAIT, K_MSEC(BATTERY_SAMPLE_INTERVAL_MS));

}

int main(void)
{
        configure_saadc();

        k_sleep(K_FOREVER);
        return 0;
}