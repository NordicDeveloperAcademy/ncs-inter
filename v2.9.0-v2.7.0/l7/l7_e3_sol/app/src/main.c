/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <zephyr/kernel.h>
 #include <blink.h>
 #include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Lesson7_Exercise3, LOG_LEVEL_INF);
 
  
 #define BLINK_PERIOD_MS_STEP 100U
 #define BLINK_PERIOD_MS_MAX  1000U
 
 int main(void)
 {
    /* Start blinking - slow*/ 
    unsigned int period_ms = BLINK_PERIOD_MS_MAX;
 
     LOG_INF("Zephyr Example Application");
 
     const struct device * blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));
     if (!device_is_ready(blink)) {
         LOG_ERR("Blink LED not ready");
         return 0;
     }

     /* STEP 5.3 Use the custom blink API from the driver to change the blinking period */
     /* Use custom API  to turn LED  off */
     int ret = blink_off(blink);
     if (ret < 0) {
         LOG_ERR("Could not turn off LED (%d)", ret);
         return 0;
     }
 
     while (1) {
        
        /* When LED is constantly enabled - start over with high blinking period*/
        if (period_ms == 0U) {
            period_ms = BLINK_PERIOD_MS_MAX;
        } else {
            period_ms -= BLINK_PERIOD_MS_STEP;
        }
 
        LOG_INF("Setting LED period to %u ms",
            period_ms);
        
        /* Use custom API to change LED blinking period*/
        blink_set_period_ms(blink, period_ms);
       

        k_sleep(K_MSEC(2000));
     }
 
     return 0;
 }
 
 