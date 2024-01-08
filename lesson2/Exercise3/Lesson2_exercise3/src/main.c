#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>


#define SLEEP_TIME_MS  1000


#define GPIO_NODE DT_NODELABEL (button0)
#define LED0_NODE	DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(GPIO_NODE, gpios);



// Toggles led0 when button is pressed 
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&led);
    printk("Toggle light \r\n");
}

 
static struct gpio_callback sw0_cb_data;

int main(void)
{
        int ret;
	printk("Lesson 2 Exersice 3 Devicetree error Started\n");
        if (!device_is_ready(sw0.port)) {
		printk("Button 1 not ready \n");
	}


	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
                printk("Error setting led0 as output \n");
		return;
	}

	ret = gpio_pin_configure_dt(&sw0, GPIO_INPUT);
	if (ret < 0) {
                printk("Error setting sw0 as input \n");
		return;
	}

        // setting up cacllbacks
        ret = gpio_pin_interrupt_configure_dt(&sw0, GPIO_INT_EDGE_TO_ACTIVE );
        gpio_init_callback(&sw0_cb_data, button_pressed, BIT(sw0.pin)); 

        gpio_add_callback(sw0.port, &sw0_cb_data);
        
        while (1) {
                k_msleep(SLEEP_TIME_MS);
	}



}
