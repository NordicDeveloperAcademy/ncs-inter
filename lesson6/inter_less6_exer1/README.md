#Exercise 1 - Introduction to adding the pwm device

## Step 1
### Step 1.1 - Enable logging
Add the required configurations to prj.conf

### Step 1.2 - Add log support
 Add the log module header to main.c and register the log module

## Step 2
### Step 2.1 - Include the relevant headers and configurations for pwm
 (device.h and drivers/pwm.h) and include the PWM configuration. You may also add CONFIG_PWM_LOG_LEVEL_DBG=y
 for additional logging.
 

### Step 2.2 - Define pwm_led0
Once a Zephyr driver is enabled by the configuration in our prj.conf file, it will automatically be enabled and ready for use. All we need to do is to specify what instance we want to use. 

Investigate the pwm_dt_spec struct at https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/hardware/peripherals/pwm.html#c.pwm_dt_spec and see how you can use this to get pwm_led0, i.e LED 1 as our pwm device. 

This line will find our pwm_leds instance and use LED1 as the default PWM pin. 

On a nRF52840DK LED1 is P0.13 so toggling the LED will also toggle that GPIO on the GPIO header on the board.

## Step 3 - Create a pwm_init function
For this basic use case you won't require a pwm initialization function, but in general it might be beneficial for later use if you aim to use PWM as a control signal for a motor. Note how this is done similarly in exercise 2 with the motor 3 as well.

Define the initialization function int pwm_init(void) which currently returns nothing

This function should 
* Step 3.2 Check if the pwm device is ready and return a busy status if it is busy
* Step 3.3 Set the led with the control signal generated from the PWM
   
### Step 3.1 
Before we create the function we need to define the PWM period and pulse. Do this by defining PWM_PERIOD_NS and PWM_PULSE_NS 

We will first select a number that can seem arbitrary, but it is picked from the hardware from excercize 2 regarding pwm motor control. We want to use this signal to see how it translates to a LED. Set the period to 20ms and a duty cycle between 1ms and 2 ms.

At step 3.5 we will set it to something that is easier to observe when testing the PWM peripheral for an LED.

Investigate pwm_set_dt to see how you can set the PWM period and pulse on the device https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/hardware/peripherals/pwm.html#c.pwm_set_dt

### Step 3.2 - Check if the device is busy
Investigate device_is_ready to see how you can check if the device is busy https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/kernel/drivers/index.html#c.device_is_ready

The PWM is running cyclically in a given period and duty cycle and works as a counter. At the start of the cycle the counter is 0 and the PWM pin starts high. When it reaches a specific value called the PWM duty cycle, the PWM signal will go low. Thus we want to check if the device is busy or not before we set it

INSERT IMAGE OF PWM PERIOD AND PWM DUTY CYCLE, PS THIS ONE IS FROM THE MOTOR SPEC

### Step 3.3 . Set the led with the control signal generated from the PWM
Use pwm_set_dt from step 3.1 to set the LED. Remember to check for error codes.

## Step 3.4 - Test your firmware
Build and flash the firmware. 

If you've used the pulse and period values we suggested in step 3.1 you should now observe that LED 1 on the 52840DK is glowing slightly. 

Try changing the pulse and/or period and observe the changes. If the value is set too high or too low, it may be out of range of what your hardware can reach. In step 4/LESSON 2 AN OPTION TO ADD MORE TO THE LESSON OR END IT HERE we will show you how you can calibrate the PWM channel to ensure that the period is within the tolerance of the hardware.

Congratulations, you've now enabled the PWM and used it to control a LED!

