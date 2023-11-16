
# Step 4 - Add period calibration to the application
In the previous steps you've seen how you can define the period and pulse manually. However, if you chose parameters similarly to what I set in the solution, you may have observed that theese parameters are not suited to control a blinking LED that a human easily observe.

Instead we will take a page out of the blinky_pwm sample found in the Zephyr repository in nRF Connect SDK and add some calibration. Doing this will also allow us to easily set a maximum period that can be set for most PWM hardware.
The goal is to set a high value using PWM_SEC, and to decrease the value until it can be set. 

Comment out 

/* STEP 3.1 - Define the desired pwm period and pulse */
#define PWM_PERIOD_NS 20000000 
#define PWM_PULSE_NS 1500000

and 

/* Step 3.3 - Set period and pulse in nanoseconds and check for error*/
err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, PWM_PULSE_NS);
if (err){
        LOG_ERR("pwm_set_dt() in pwm_init() returned %d", err);
}

## Step 4.1.1 
Define a minimum and maximum period using PWM_SEC above pwm_init(). It can be set to (1U)/128U for minimum and (1U) for maximum. We will modify this value in a later step.

## Step 4.1.2
Define max_period in pwm_init()

## Step 4.2
In pwm_init, set max_period = MAX_PERIOD and decrease it as long as pwm_set_dt(&pwm_led0, max_period,max_period/2U)

## Step 4.3
In this optional step you may make sure that the maximum period is at least 4 times higher than MIN_PERIOD to make sure the sample changes frequency at least once

## Step 4.4 
Return max_period from pwm_init()

## Step 4.5 
Declare an unsigned 32bit int "max_period", "period" and unsigned 8 bit int "dir". Set dir to be equal to 0. Set max_period to become the returned value of pwm_init() and set period to be equal to max_period.

We want to use the variable "dir" to indicate which direction 

## Step 4.6
In a while true loop, use pwm_set_dt(&pwm_led0, period, period / 2U) to toggle the light, and change the period in intervals. Make sure that period stays less than max_period and larger than MIN_PERIOD