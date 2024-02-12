#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(Lesson4_Exercise2, LOG_LEVEL_INF);

#define PWM_LED0        DT_ALIAS(pwm_led0)

/* STEP 4.7 - Define the servo motor through its nodelabel */
   

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(PWM_LED0);

/* STEP 5.4 - Retrieve the device structure for the servo motor */



/* STEP 5.5 - Use DT_PROP() to obtain the minimum and maximum duty cycle */


#define PWM_PERIOD   PWM_MSEC(20)

/* STEP  2.2 - Define minimum and maximum duty cycle */
/* STEP 4.2 - Change the duty cycles for the LED */


/* STEP 2.1 - Create a function to set the angle of the motor */


/* STEP 4.3 - Create a function to set the duty cycle of a PWM LED */

void button_handler(uint32_t button_state, uint32_t has_changed)
{
    int err = 0;
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
            /* STEP 2.4 - Change motor angle when a button is pressed */
            /* STEP 5.6 - Update the button handler with the new duty cycle */

            /* STEP 4.4 - Change LED when a button is pressed */

		}
        if (err) {
            LOG_ERR("Error: couldn't set duty cycle, err %d", err);
        }
	}
}

int main(void)
{

    int err = 0;
        
    if (dk_buttons_init(button_handler)) {
        LOG_ERR("Failed to initialize the buttons library");
    }

    /* STEP 2.3 - Check if the device is ready and set its initial value */


    /* STEP 5.7 - Check if the motor device is ready and set its initial value */


    return 0;
}
