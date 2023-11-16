#include <zephyr/kernel.h>
/* Step 2.1 - Include the relevant headers for pwm*/

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

/* Step 1.2 - Include logging and define log module */
#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/* Step 2.2 - define pwm_led0 using the pwm_dt_spec struct */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

/* STEP 3.1 - Define the desired pwm period and pulse */
// #define PWM_PERIOD_NS 20000000
// #define PWM_DUTY_CYCLE 1400000

/* Step  3.4 - With different parameters */
#define PWM_PERIOD_NS 100000000
#define PWM_DUTY_CYCLE 14000000

/* Step 3 - Create an initializing function to verify that the device is enabled*/ 
int pwm_init(void)
{
    int err = 0;
    LOG_INF("Initializing Motor Control");

/* Step 3.2 - Check if the device is ready */
    if (!device_is_ready(pwm_led0.dev)) {
    LOG_ERR("Error: PWM device %s is not ready",
            pwm_led0.dev->name);
    return -EBUSY;
	}
	
/* Step 3.3 - Set period and pulse in nanoseconds and check for error*/
    err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, PWM_DUTY_CYCLE);
    if (err) {
        LOG_ERR("pwm_set_dt returned %d", err);
    }

    return err;
}

void main(void)
{
        LOG_INF("Starting PWM basic sample \n");
        /* Step 3.3 - Call the initialization function */
        pwm_init();
        
}
