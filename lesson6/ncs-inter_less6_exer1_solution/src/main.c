#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* STEP 1.3 - Include the relevant headers for pwm*/
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(Lesson6_Exercise1, LOG_LEVEL_INF);

/* STEP 2 - Define the desired PWM period and pulse */
 #define PWM_PERIOD_NS 20000000
 #define PWM_DUTY_CYCLE 1400000

/* STEP 3.1 - Get the node identifier for [] through its alias */
#define PWM_LED0    DT_ALIAS(pwm_led0)

/* STEP 3.2 - Initialize and populate struct pwm_dt_spec */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(PWM_LED0);

void main(void)
{

    /* STEP 3.3 - Check if the device is ready */
    if (!device_is_ready(pwm_led0.dev)) {
        LOG_ERR("PWM device %s is not ready", pwm_led0.dev->name);
        return -EBUSY;
	}
    
    /* STEP 4 - Control the LED with the control signal generated from the PWM */
    err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, PWM_DUTY_CYCLE);
    if (err) {
        LOG_ERR("pwm_set_dt returned %d", err);
    }    
}
