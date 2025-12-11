/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Lesson6_Exercise3, LOG_LEVEL_DBG);

/* STEP 2 - Include header for nrfx drivers */
#include <nrfx_saadc.h>
#include <nrfx_timer.h>
#include <helpers/nrfx_gppi.h>

/* STEP 3.1 - Define the SAADC sample interval in microseconds */
#define SAADC_SAMPLE_INTERVAL_US 50

/* STEP 4.1 - Define the buffer size for the SAADC */
#define SAADC_BUFFER_SIZE   8000

/* STEP 4.6 - Declare the struct to hold the configuration for the SAADC channel used to sample the battery voltage */
#if NRF_SAADC_HAS_AIN_AS_PIN
#if defined(CONFIG_SOC_NRF54L15 ) || defined(CONFIG_SOC_NRF54LM20A)
#define SAADC_INPUT_PIN NRFX_ANALOG_EXTERNAL_AIN4
#else
BUILD_ASSERT(0, "Unsupported device family");
#endif
#else 
#define SAADC_INPUT_PIN NRFX_ANALOG_EXTERNAL_AIN0 
#endif

static nrfx_saadc_channel_t channel = NRFX_SAADC_DEFAULT_CHANNEL_SE(SAADC_INPUT_PIN, 0);


/* STEP 3.2 - Declaring an instance of nrfx_timer for TIMER2. */
#if defined(CONFIG_SOC_NRF54L15) || defined(CONFIG_SOC_NRF54LM20A)
#define TIMER_INSTANCE_NUMBER NRF_TIMER22
#else
#define TIMER_INSTANCE_NUMBER NRF_TIMER2
#endif
nrfx_timer_t timer_instance = NRFX_TIMER_INSTANCE(TIMER_INSTANCE_NUMBER);

/* STEP 4.2 - Declare the buffers for the SAADC */
static int16_t saadc_sample_buffer[2][SAADC_BUFFER_SIZE];

/* STEP 4.3 - Declare variable used to keep track of which buffer was last assigned to the SAADC driver */
static uint32_t saadc_current_buffer = 0;

static void configure_timer(void)
{
    nrfx_err_t err;

    /* STEP 3.3 - Declaring timer config and intialize nrfx_timer instance. */
    nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG(1000000);
    err = nrfx_timer_init(&timer_instance, &timer_config, NULL);
    if (err != 0) {
        LOG_ERR("nrfx_timer_init error: %08x", err);
        return;
    }

    /* STEP 3.4 - Set compare channel 0 to generate event every SAADC_SAMPLE_INTERVAL_US. */
    uint32_t timer_ticks = nrfx_timer_us_to_ticks(&timer_instance, SAADC_SAMPLE_INTERVAL_US);
    nrfx_timer_extended_compare(&timer_instance, NRF_TIMER_CC_CHANNEL0, timer_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);

}

static void saadc_event_handler(nrfx_saadc_evt_t const * p_event)
{
    nrfx_err_t err;
    switch (p_event->type)
    {
        case NRFX_SAADC_EVT_READY:
        
           /* STEP 5.1 - Buffer is ready, timer (and sampling) can be started. */
            nrfx_timer_enable(&timer_instance);
            break;                        
            
        case NRFX_SAADC_EVT_BUF_REQ:
        
            /* STEP 5.2 - Set up the next available buffer. Alternate between buffer 0 and 1 */
            err = nrfx_saadc_buffer_set(saadc_sample_buffer[(saadc_current_buffer++)%2], SAADC_BUFFER_SIZE);
            //err = nrfx_saadc_buffer_set(saadc_sample_buffer[((saadc_current_buffer == 0 )? saadc_current_buffer++ : 0)], SAADC_BUFFER_SIZE);
            if (err != 0) {
                LOG_ERR("nrfx_saadc_buffer_set error: %08x", err);
                return;
            }
            break;

        case NRFX_SAADC_EVT_DONE:

            /* STEP 5.3 - Buffer has been filled. Do something with the data and proceed */
            int64_t average = 0;
            int16_t max = INT16_MIN;
            int16_t min = INT16_MAX;
            int16_t current_value; 
            for(int i=0; i < p_event->data.done.size; i++){
                current_value = ((int16_t *)(p_event->data.done.p_buffer))[i];
                average += current_value;
                if(current_value > max){
                    max = current_value;
                }
                if(current_value < min){
                    min = current_value;
                }
            }
            average = average/p_event->data.done.size;
            LOG_INF("SAADC buffer at 0x%x filled with %d samples", (uint32_t)p_event->data.done.p_buffer, p_event->data.done.size);
            LOG_INF("AVG=%d, MIN=%d, MAX=%d", (int16_t)average, min, max);
            break;
        default:
            LOG_INF("Unhandled SAADC evt %d", p_event->type);
            break;
    }
}

static void configure_saadc(void)
{
    nrfx_err_t err;

    /* STEP 4.4 - Connect ADC interrupt to nrfx interrupt handler */
    IRQ_CONNECT(DT_IRQN(DT_NODELABEL(adc)),
                DT_IRQ(DT_NODELABEL(adc), priority),
                nrfx_isr, nrfx_saadc_irq_handler, 0);

    
    /* STEP 4.5 - Initialize the nrfx_SAADC driver */
    err = nrfx_saadc_init(DT_IRQ(DT_NODELABEL(adc), priority));
    if (err != 0) {
        LOG_ERR("nrfx_saadc_init error: %08x", err);
        return;
    }

    /* STEP 4.7 - Change gain config in default config and apply channel configuration */
#if defined(CONFIG_SOC_NRF54L15) || defined(CONFIG_SOC_NRF54LM20A)
    channel.channel_config.gain = NRF_SAADC_GAIN1_4;
#else
    channel.channel_config.gain = NRF_SAADC_GAIN1_6;
#endif
    err = nrfx_saadc_channels_config(&channel, 1);
    if (err != 0) {
        LOG_ERR("nrfx_saadc_channels_config error: %08x", err);
        return;
    }

    /* STEP 4.8 - Configure channel 0 in advanced mode with event handler (non-blocking mode) */
    nrfx_saadc_adv_config_t saadc_adv_config = NRFX_SAADC_DEFAULT_ADV_CONFIG;
    err = nrfx_saadc_advanced_mode_set(BIT(0),
                                        NRF_SAADC_RESOLUTION_12BIT,
                                        &saadc_adv_config,
                                        saadc_event_handler);
    if (err != 0) {
        LOG_ERR("nrfx_saadc_advanced_mode_set error: %08x", err);
        return;
    }
                                            
    /* STEP 4.9 - Configure two buffers to make use of double-buffering feature of SAADC */
    err = nrfx_saadc_buffer_set(saadc_sample_buffer[0], SAADC_BUFFER_SIZE);
    if (err != 0) {
        LOG_ERR("nrfx_saadc_buffer_set error: %08x", err);
        return;
    }
    err = nrfx_saadc_buffer_set(saadc_sample_buffer[1], SAADC_BUFFER_SIZE);
    if (err != 0) {
        LOG_ERR("nrfx_saadc_buffer_set error: %08x", err);
        return;
    }

    /* STEP 4.10 - Trigger the SAADC. This will not start sampling, but will prepare buffer for sampling triggered through PPI */
    err = nrfx_saadc_mode_trigger();
    if (err != 0) {
        LOG_ERR("nrfx_saadc_mode_trigger error: %08x", err);
        return;
    }

}

static void configure_ppi(void)
{
    nrfx_err_t err;
    /* STEP 6.1 - Declare variables used to hold the (D)PPI channel number */
    nrfx_gppi_handle_t gppi_handle_sample;
    nrfx_gppi_handle_t gppi_handle_start;

    /* STEP 6.2 - Trigger task sample from timer */
    err = nrfx_gppi_conn_alloc( nrfx_timer_compare_event_address_get(&timer_instance, NRF_TIMER_CC_CHANNEL0),nrf_saadc_task_address_get(NRF_SAADC, NRF_SAADC_TASK_SAMPLE), &gppi_handle_sample);
    if (err != 0) {
        LOG_ERR("nrfx_gppi_conn_alloc error: %08x", err);
        return;
    }

    /* STEP 6.3 - Trigger task start from end event */
    err = nrfx_gppi_conn_alloc(nrf_saadc_event_address_get(NRF_SAADC, NRF_SAADC_EVENT_END),nrf_saadc_task_address_get(NRF_SAADC, NRF_SAADC_TASK_START), &gppi_handle_start);
    if (err != 0) {
        LOG_ERR("nrfx_gppi_conn_alloc error: %08x", err);
        return;
    }

    /* STEP 6.4 - Enable both (D)PPI channels */ 
    nrfx_gppi_conn_enable(gppi_handle_sample);
    nrfx_gppi_conn_enable(gppi_handle_start);
}


int main(void)
{
    configure_timer();
    configure_saadc();  
    configure_ppi();
    k_sleep(K_FOREVER);
}
