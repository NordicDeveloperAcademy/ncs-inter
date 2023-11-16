This exercise will show you how to use Zephyrs API to get and use a predefined pwm compatible node on the nRF52840. 

In the GitHub repository for this course, open the base code for this exercise, found in lesson6/inter_less6_exer1.

Exercise Steps:

In the GitHub repository for this course, open the base code for this exercise, found in lesson6/inter_less1_exer1.

Step 1 - Add logging and printk support to your project by adding the following to prj.conf

1.0 

#Step 1.1 - Enable logging
CONFIG_LOG=y
CONFIG_PWM_LOG_LEVEL_DBG=y
CONFIG_LOG_PRINTK=y
CONFIG_LOG_MODE_IMMEDIATE=y
CONFIG_STDOUT_CONSOLE=y
CONFIG_PRINTK=y

1.1 In main.c add under 1.1 

#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

Step 2 - Add the pwm module, relevant headers and define the pwm_led device

2.0 - In prj.conf add 

#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

2.1 - In main.c include the following headers

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

2.2 - Define pwm_led0 using the pwm_dt_spec struct in main.c

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

Step 3 - Create a pwn_init() function

For this basic use case you won't require a pwm initialization function, but in general it might be beneficial for later use if you aim to use PWM as a control signal for a motor. Note how this is done similarly in exercise 2 with the motor.

Define the initialization function int pwm_init(void) which returns an error code. 

This function should

Step 3.2 Check if the pwm device is ready and return a busy status if it is busy

Step 3.3 Set the led with the control signal generated from the PWM

3.0 - Enable LED and LED_PWM configuration
Add the following to prj.conf

CONFIG_LED=y
CONFIG_LED_PWM=y

3.1 - Set the PWM period and pulse

Before we create the function we need to define the PWM period and pulse. Do this by defining PWM_PERIOD_NS and PWM_PULSE_NS 

Investigate pwm_set_dt to see how you can set the PWM period and pulse on the device https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/hardware/peripherals/pwm.html#c.pwm_set_dt 


If we take a look at the datasheet for the motor we will use in exercize 2 we can see that the motors PWM period is 20ms (50Hz) and that the duty cycle is in between 1ms and 2 ms. So lets use the same parameters for now. We will change them later in this exercize.

Near the top of main.c add 

#define PWM_PERIOD_NS 20000000
#define PWM_DUTY_CYCLE 1400000

3.2 - Check if the device is busy

Investigate device_is_ready to see how you can check if the device is busy https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/drivers/index.html#c.device_is_ready

Under Step 3.2 in main.c add

    if (!device_is_ready(pwm_led0.dev)) {
    LOG_ERR("Error: PWM device %s is not ready",
            pwm_led0.dev->name);
    return -EBUSY;
	}

3.3 - Control the LED with the control signal generated from the PWM

Use pwm_set_dt from step 3.1 to set the LED. Remember to check for error codes.



3.4 - Test your firmware

Build and flash the firmware. LED 1 on the 52840DK should glow slightly. Try changing the pulse and/or period and observe the changes. If the value is set too high or too low, it may be out of range of what your hardware can reach. For instance set the parameters to 

#define PWM_PERIOD_NS 100000000
#define PWM_DUTY_CYCLE 14000000

Congratulations, you've now enabled the PWM and used it to control a LED!

Step 4 -  Optional addition to the exercize by investigating existing samples

For those of you who wish to have a look at how you can add to this very basic exercize, you can have a look at lesson6/inter_less1_exer1_opt. This folder takes what we’ve done so far in exercize 1 and adds functionality from the pwm_led sample found in the zephyr sample folder in NCS and breaks it down into steps to make it easier for users to see how this implementation has been done.
