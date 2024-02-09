/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* Step 3 - Include header for nrfx drivers */


/* Step 4.1 - Define the SAADC sample interval in microseconds */


/* Step 5.1 - Define the buffer size for the SAADC */


/* Step 4.2 - Declaring an instance of nrfx_timer for TIMER2. */


/* Step 5.2 - Declare the buffers for the SAADC */


/* Step 5.3 - Declare variable used to keep track of which buffer was last assigned to the SAADC driver */


static void configure_timer(void)
{
    nrfx_err_t err;

    /* Step 4.3 - Declaring timer config and intialize nrfx_timer instance. */


    /* Step 4.4 - Set compare channel 0 to generate event every SAADC_SAMPLE_INTERVAL_US. */


}

static void saadc_event_handler(nrfx_saadc_evt_t const * p_event)
{
    nrfx_err_t err;
    switch (p_event->type)
    {
        case NRFX_SAADC_EVT_READY:
        
           /* Step 6.1 - Buffer is ready, timer (and sampling) can be started. */


            break;                        
            
        case NRFX_SAADC_EVT_BUF_REQ:
        
            /* Step 6.2 - Set up the next available buffer. Alternate between buffer 0 and 1 */


            break;

        case NRFX_SAADC_EVT_DONE:

            /* Step 6.3 - Buffer has been filled. Do something with the data and proceed */


            break;

        default:
            LOG_INF("Unhandled SAADC evt %d", p_event->type);
            break;
    }
}

static void configure_saadc(void)
{
    nrfx_err_t err;

    /* Step 5.4 - Connect ADC interrupt to nrfx interrupt handler */


    
    /* Step 5.5 - Initialize the nrfx_SAADC driver */


    
    /* Step 5.6 - Declare the struct to hold the configuration for the SAADC channel used to sample the battery voltage */


    /* Step 5.7 - Change gain config in default config and apply channel configuration */


    /* Step 5.8 - Configure channel 0 in advanced mode with event handler (non-blocking mode) */

                                            
    /* Step 5.9 - Configure two buffers to make use of double-buffering feature of SAADC */


    /* Step 5.10 - Trigger the SAADC. This will not start sampling, but will prepare buffer for sampling triggered through PPI */


}

static void configure_ppi(void)
{
    nrfx_err_t err;
    /* Step 7.1 - Declare variables used to hold the (D)PPI channel number */


    /* Step 7.2 - Trigger task sample from timer */



    /* Step 7.3 - Trigger task sample from timer */


    /* Step 7.4 - Trigger task start from end event */


    /* Step 7.5 - Enable both (D)PPI channels */ 

}


int main(void)
{
    configure_timer();
    configure_saadc();  
    configure_ppi();
    k_sleep(K_FOREVER);
}
