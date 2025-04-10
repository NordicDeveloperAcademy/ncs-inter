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
     int ret;
     unsigned int period_ms = BLINK_PERIOD_MS_MAX;
 
     printk("Zephyr Example Application \n");
 
     const struct device * blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));
     if (!device_is_ready(blink)) {
         LOG_ERR("Blink LED not ready");
         return 0;
     }
 
     ret = blink_off(blink);
     if (ret < 0) {
         LOG_ERR("Could not turn off LED (%d)", ret);
         return 0;
     }
 
     printk("Use the sensor to change LED blinking period\n");
 
     while (1) {
   
        if (period_ms == 0U) {
            period_ms = BLINK_PERIOD_MS_MAX;
        } else {
            period_ms -= BLINK_PERIOD_MS_STEP;
        }
 
        printk("Setting LED period to %u ms\n",
            period_ms);

        blink_set_period_ms(blink, period_ms);

        k_sleep(K_MSEC(2000));
     }
 
     return 0;
 }
 
 