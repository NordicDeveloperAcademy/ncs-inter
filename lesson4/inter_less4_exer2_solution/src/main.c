#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(Lesson4_Exercise2, LOG_LEVEL_INF);

#define PWM_LED0        DT_ALIAS(pwm_led0)

/* STEP 4.7 - Define the servo motor through its nodelabel */
#define SERVO_MOTOR     

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(PWM_LED0);

/* STEP 5.4 - Retrieve the device structure for the servo motor */
static const struct pwm_dt_spec pwm_servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));


/* STEP 5.5 - Use DT_PROP() to obtain the minimum and maximum duty cycle */
#define PWM_SERVO_MIN_DUTY_CYCLE    DT_PROP(SERVO_MOTOR, min_pulse)
#define PWM_SERVO_MAX_DUTY_CYCLE    DT_PROP(SERVO_MOTOR, max_pulse)

#define PWM_PERIOD   PWM_MSEC(20)

/* STEP  2.2 - Define minimum and maximum duty cycle */
/* STEP 4.2 - Change the duty cycles for the LED */
#define PWM_MIN_DUTY_CYCLE 20000000
#define PWM_MAX_DUTY_CYCLE 50000000

/* STEP 2.1 - Create a function to set the angle of the motor */
int set_motor_angle(uint32_t duty_cycle_ns)
{
    int err;
    
    err = pwm_set_dt(&pwm_servo, PWM_PERIOD, duty_cycle_ns);
    if (err) {
        LOG_ERR("pwm_set_dt_returned %d", err);
    }
    return err;
}
/* STEP 4.3 - Create a function to set the duty cycle of a PWM LED */
int set_led_blink(uint32_t period, uint32_t duty_cycle_ns){
    int err;
    err = pwm_set_dt(&pwm_led0, period, duty_cycle_ns);
        if (err) {
        LOG_ERR("pwm_set_dt_returned %d", err);
    }
    return err;
}

void button_handler(uint32_t button_state, uint32_t has_changed)
{
    int err = 0;
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
            /* STEP 2.4 - Change motor angle when a button is pressed */
            /* STEP 5.6 - Update the button handler with the new duty cycle */
            case DK_BTN1_MSK:
                LOG_INF("Button 1 pressed");
                err = set_motor_angle(PWM_SERVO_MIN_DUTY_CYCLE);
                break;
            case DK_BTN2_MSK:
                LOG_INF("Button 2 pressed");
                err = set_motor_angle(PWM_SERVO_MAX_DUTY_CYCLE);
                break;
            /* STEP 4.4 - Change LED when a button is pressed */
            case DK_BTN3_MSK:
                LOG_INF("Button 3 pressed");
                err = set_led_blink(2*PWM_PERIOD, PWM_MIN_DUTY_CYCLE);
                break;
            case DK_BTN4_MSK:
                LOG_INF("Button 4 pressed");
                err = set_led_blink(4*PWM_PERIOD, PWM_MAX_DUTY_CYCLE);
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
        
    if (dk_buttons_init(button_handler)) {
        LOG_ERR("Failed to initialize the buttons library");
    }

    /* STEP 2.3 - Check if the device is ready and set its initial value */
    if (!pwm_is_ready_dt(&pwm_led0)) {
        LOG_ERR("Error: PWM device %s is not ready", pwm_led0.dev->name);
        return 0;
	}
    err = pwm_set_dt(&pwm_led0, 2*PWM_PERIOD, PWM_MIN_DUTY_CYCLE);
    if (err) {
        LOG_ERR("pwm_set_dt returned %d", err);
        return 0;
    }

    /* STEP 5.7 - Check if the motor device is ready and set its initial value */
    LOG_INF("Setting initial motor");
    if (!pwm_is_ready_dt(&pwm_servo)) {
        LOG_ERR("Error: PWM device %s is not ready", pwm_servo.dev->name);
        return 0;
	}

    err = pwm_set_dt(&pwm_servo, PWM_PERIOD, PWM_SERVO_MIN_DUTY_CYCLE);
    if (err) {
        LOG_ERR("pwm_set_dt returned %d", err);
        return 0;
    }

    return 0;
}
