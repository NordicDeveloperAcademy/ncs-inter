#include <zephyr/kernel.h>
/* Step 2.1 from excersize 1 - Include the relevant headers for pwm*/

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

/* Step 1.2 from excersize 1 - Include logging and define log module */
#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* Step 2.2 from excersize 1 - define pwm_led0 using the pwm_dt_spec struct */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));


/* Step 4.1 - Define the minimum and maximum perios using PWM_SEC  */
#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)



/* Step 3 from excersize 1 - Create an initializing function to verify that the device is enabled*/ 
int pwm_init(void)
{
    LOG_INF("Initializing Motor Control");

    if (!device_is_ready(pwm_led0.dev)) {
    LOG_ERR("Error: PWM device %s is not ready",
            pwm_led0.dev->name);
    return -EBUSY;
	}

    
    return 0;
}

int main(void)
{
    uint32_t max_period;
	uint32_t period;
	uint8_t dir = 0U;
    int err = 0;

    LOG_INF("Starting PWM basic sample \n");


    /* Step 3.3 - Call the initialization function */
    pwm_init();

    /* Step 4.2 - */
    /*
	 * In case the default MAX_PERIOD value cannot be set for
	 * some PWM hardware, decrease its value until it can.
	 *
	 * Keep its value at least MIN_PERIOD * 4 to make sure
	 * the sample changes frequency at least once.
	 */
	printk("Calibrating for channel %d...\n", pwm_led0.channel);
        max_period = MAX_PERIOD;
        
	while (pwm_set_dt(&pwm_led0, max_period, max_period / 2U)) {
		max_period /= 2U;
		if (max_period < (4U * MIN_PERIOD)) {
			printk("Error: PWM device "
			       "does not support a period at least %lu\n",
			       4U * MIN_PERIOD);
			return 0;
		}
	}

	printk("Done calibrating; maximum/minimum periods %u/%lu nsec\n",
    max_period, MIN_PERIOD);

	period = max_period;
    
        while(1)
        {
        /*Step 4.3 - In a loop use pwm_set_dt to control the LED */
        err =  pwm_set_dt(&pwm_led0, period, period / 2U);
		if (err) {
			printk("Error %d: failed to set pulse width\n", err);
			return 0;
		}
        /* Step 4.4 - Implement your own logic or take inspiration from zephyrs pwm_blinky or pwm_led sample to vary the interval of the pwm controlled LED*/
		period = dir ? (period * 2U) : (period / 2U);
		if (period > max_period) {
			period = max_period / 2U;
			dir = 0U;
		} else if (period < MIN_PERIOD) {
			period = MIN_PERIOD * 2U;
			dir = 1U;
		}

		k_sleep(K_SECONDS(4U));
        }
        return 0;
}
