#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

/* STEP 4.1.2 - Include the header file for the DK Buttons and LEDs library */
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(Lesson6_Exercise2, LOG_LEVEL_INF);

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

#define PWM_PERIOD_NS 10000000
#define PWM_DUTY_CYCLE 1400000

/* STEP  2.2 - Define minimum and maximum duty cycle */
#define PWM_MIN_DUTY_CYCLE 600000
#define PWM_MAX_DUTY_CYCLE 2000000

/* STEP 2.1 - Create a function to set the angle of the motor*/
int set_motor_angle(uint32_t duty_cycle_ns)
{
    int err;
    err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, duty_cycle_ns);
    if (err) {
        LOG_ERR("pwm_set_dt_returned %d", err);
    }
 
 	err = pwm_capture_cycles(in.dev, in.pwm, PWM_CAPTURE_TYPE_PULSE,
				 &period, &pulse, K_MSEC(1000));
    return err;
}

void button_handler(uint32_t button_state, uint32_t has_changed)
{
    int err = 0;
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
            /* STEP 4.2 - Change motor angle when a button is pressed */
            case DK_BTN1_MSK:
                LOG_INF("Button 1 pressed");
                err = set_motor_angle(PWM_MIN_DUTY_CYCLE);
                break;
            case DK_BTN2_MSK:
                LOG_INF("Button 2 pressed");
                err = set_motor_angle(PWM_MAX_DUTY_CYCLE);
                break;
			default:
				break;
		}
        if (err) {
            LOG_ERR("Error: couldn't set duty cycle, err %d", err);
        }
	}
}

void main(void)
{

    int err = 0;

    if (!pwm_is_ready_dt(&pwm_led0)) {
        LOG_ERR("Error: PWM device %s is not ready", pwm_led0.dev->name);
        return 0;
	}

    err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, PWM_DUTY_CYCLE);
    if (err) {
        LOG_ERR("pwm_set_dt returned %d", err);
        return 0;
    }

    if (dk_buttons_init(button_handler)) {
        LOG_ERR("Couldn't init buttons (err %d)", err);
    }

    for (;;){
        /* STEP 2.3 - Call set_motor_angle() with different duty cycles */
        LOG_INF("Setting motor angle to %d",PWM_MIN_DUTY_CYCLE);
        set_motor_angle(PWM_MIN_DUTY_CYCLE);
        k_sleep(K_SECONDS(5));
        LOG_INF("Setting motor angle to %d",PWM_MAX_DUTY_CYCLE);
        set_motor_angle(PWM_MAX_DUTY_CYCLE);
        k_sleep(K_SECONDS(5));
    
    }
      
}
