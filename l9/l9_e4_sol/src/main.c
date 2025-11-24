/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Step 5.4 - Include header for usb */
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <sample_usbd.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(usb_cdc, LOG_LEVEL_INF);

/* 1000 msec = 1 sec */

#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

const struct device *const uart_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);

static struct usbd_context *sample_usbd;

#define DEVICE_DT_GET_COMMA(node_id) DEVICE_DT_GET(node_id),


static void sample_msg_cb(struct usbd_context *const ctx, const struct usbd_msg *msg)
{
    LOG_INF("USBD message: %s", usbd_msg_type_string(msg->type));

    if (usbd_can_detect_vbus(ctx)) {
        if (msg->type == USBD_MSG_VBUS_READY) {
            if (usbd_enable(ctx)) {
                LOG_ERR("Failed to enable device support");
            }
        }

        if (msg->type == USBD_MSG_VBUS_REMOVED) {
            if (usbd_disable(ctx)) {
                LOG_ERR("Failed to disable device support");
            }
        }
    }
}


int main(void)
{
    int ret;

    /* Step 5.5 - Enable USB */
    sample_usbd = sample_usbd_init_device(sample_msg_cb);
    if (sample_usbd == NULL) {
        LOG_ERR("Failed to initialize USB device");
        return -ENODEV;
    }

    if (!usbd_can_detect_vbus(sample_usbd)) {
        ret = usbd_enable(sample_usbd);
        if (ret) {
            LOG_ERR("Failed to enable device support");
            return ret;
        }
    }

    LOG_INF("USB device support enabled");

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    while (1) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            return 0;
        }
        k_msleep(SLEEP_TIME_MS);
    }
    return 1;
}
