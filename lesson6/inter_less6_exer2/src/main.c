/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* STEP 3 - Include header for nrfx SAADC driver */


/* STEP 4.1 - Declare the struct to hold the configuration for the SAADC channel used to sample the battery voltage */


/* STEP 4.2 - Declare the buffer to hold the SAAD sample value */


/* STEP 5.1 - Define the battery sample interval */


/* STEP 5.3 - Add forward declaration of timer callback handler */


/* STEP 5.2 - Define the battery sample timer instance */


/* STEP 8.1 - Implement timer callback handler function */


static void configure_saadc(void)
{
        /* STEP 6.1 - Connect ADC interrupt to nrfx interrupt handler */

        
        /* STEP 6.2 - Connect ADC interrupt to nrfx interrupt handler */


        /* STEP 6.3 - Configure the SAADC channel */


        /* STEP 6.4 - Configure nrfx_SAADC driver in simple and blocking mode */


        /* STEP 6.5 - Set buffer where sample will be stored */


        /* STEP 7 - Start periodic timer for battery sampling */


}

int main(void)
{
        configure_saadc();

        k_sleep(K_FOREVER);
        return 0;
}
