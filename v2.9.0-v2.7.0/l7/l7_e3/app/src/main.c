/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <zephyr/kernel.h>
 #include <zephyr/logging/log.h>
 #include <blink.h>
  
 LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);
 
 #define BLINK_PERIOD_MS_STEP 100U
 #define BLINK_PERIOD_MS_MAX  1000U
 
 int main(void)
 {
    /* Start blinking - slow*/ 
    unsigned int period_ms = BLINK_PERIOD_MS_MAX;
 
     printk("Zephyr Example Application \n");
 
     const struct device * blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));
     if (!device_is_ready(blink)) {
         LOG_ERR("Blink LED not ready");
         return 0;
     }

     /* STEP 5.3 Use the custom blink API from the driver to change the blinking period */

 
     return 0;
 }
 
 