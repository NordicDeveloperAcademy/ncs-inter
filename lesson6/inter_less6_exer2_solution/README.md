In this excercise we will expand on exercise 1, and it is not a problem if you’ve not completed the optional step 4. The starting point for this exercise is found in lesson6/inter_less6_exer2.

For this exercise we will use the  MG90S metal gear servo motor, but you can follow along without any additional hardware than the DK if you would like to. The first thing we will do is to use the pwm_led instance we used in the first exercise and create a custom variation of this that we will use for our PWM signal that will drive another GPIO than LED 1. If you don’t have a motor available, simply follow along with the steps in this exercise but don’t change the GPIO that this PWM instance will drive.

The data sheet for the servo motor we will use can be found here http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf

From the datasheet we can see that the PWM period is 20ms (50Hz) and the duty cycle is in between 1ms and 2ms. If you did exercise 1, you may remember that these values are the ones we started the exercise with when we tested the PWM peripheral for controlling LED1, and we observed that these values only resulted in a dimly lit LED. For the motor on the other hand, the values must be within this.

The servo can rotate approximately 90 degrees in each direction. 1ms pulse corresponds to -90 degrees, 1.5ms corresponds to 0 degrees and 2ms corresponds to 90 degrees rotation.

In lesson 1 we used LED1 as our PWM LED, which is connected to p0.13. We need to use one of the available GPIOs for our motor, and to do this we need to make some modifications to the device tree.



Step 1 - Create and modify an .overlay file

A good starting point when learning about devicetree is to inspect a boards devicetree. After you’ve built exercise 1, you can inspect the nRF52840dk_nrf52840.dts located under Input files and see how things are configured here.



Image 1: nRF52840DK default devicetree



Do note that we will not make our modifications here, and instead we will make the modifications in an overlay file which is specific for this project. To learn more about overlay files, see [THIS LESSON AND OR THESE LINKS]

Here you see compatibles such as the predefined pwmled, buttons and more.

In the pwm_led compatible we can also see that PWM_POLARITY_INVERTED is set. For our motor control we wish to have a normal polarity so we will modify that as well.

Don’t close this file as we will be using items from here by copying and adding them to our overlay file.

1.1 Create an .overlay file

In the template for this exercise we have provided you with a premade overlay file so you will not create one for this exercise, but in general if you wish to modify a device tree you will need to create one yourself. In that case you will need to create a file in your projects folder and name it "<your_board>.overlay". If you're using a nRF52840DK it will be "nrf52840dk_nrf52840.overlay". This file can be placed in your top level project folder.

In Image 1 above you can see that the overlay file we have created will place itself in the devicetree folder located under the Config files item in the extension. 

1.2 - Create your own custom compatible in your overlay file

Copy the pwm_led instance from nrf52840dk_nrf52840.dts line 48-53 and paste it into your projects overlay file under step 1.2. 

We also want to change the polarity to normal. This means that the PWM signal will have a high output for the duty cycle instead of low.

This is how it should look under step 1.2 in your overlay file

/{
    pwmleds {
        compatible = "pwm-leds";
        pwm_led0: pwm_led_0 {
            pwms = <&pwm0 0 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
        };
    };
};


1.3 - Add your own custom pwm0 instance

In your overlay file right click “&pwm0” in the instance of pwm_leds in and click "go to definition" in the device tree. This will take you to the definition in the devicetree file we had a look at earlier and you will see something like this:



Image 2: &pwm0 pin control

&pwm0 {
	status = "okay";
	pinctrl-0 = <&pwm0_default>;
	pinctrl-1 = <&pwm0_sleep>;
	pinctrl-names = "default", "sleep";
};

Observe that the pin numbers are set in pmw0_default and pwm0_sleep. Now copy the &pwm0 instance into your overlay under step 1.3.

We want to create our own custom instance of this so we need to change the names. You can choose other names, but this is what I’ve chosen

&pwm0 {
    status = "okay";
    pinctrl-0 = <&pwm0_custom>;
    pinctrl-1 = <&pwm0_sleep>;
    pinctrl-names = "default", "sleep";
};

1.4 - Set which pins that your custom pwm0 instance should use

Inspect “pwm0_default” and “pwm0_sleep” in the initial devicetree file by right_clicking and click “go to definition”. It should look like this by default:



Image 3: pwm0 pin select

&pinctrl {
	pwm0_default: pwm0_default {
		group1 {
			psels = <NRF_PSEL(PWM_OUT0, 0, 13)>;
			nordic,invert;
		};
	};
    
	pwm0_sleep: pwm0_sleep {
		group1 {
			psels = <NRF_PSEL(PWM_OUT0, 0, 13)>;
			low-power-enable;
		};
	};
};

Now copy this into your overlayfile and change “pwm0_default” and “pwm0_sleep” to the custom name you chose. It should look like this:

&pinctrl {
    pwm0_custom: pwm0_custom {
        group1 {
            psels = <NRF_PSEL(PWM_OUT0, 0, 13)>;
            nordic,invert;
        };
    };

    pwm0_csleep: pwm0_csleep {
        group1 {
            psels = <NRF_PSEL(PWM_OUT0, 0, 13)>;
            low-power-enable;
        };
    };
};

You can have a look at how NRF_PSEL works by inspecting its definition. Currently pwm0 is set to use port 0, pin 13 for its output.

Since pin 13 us used by LED 1 we need to change the output pin if we want to control anything else than the motor. In case you don’t have a motor available, you can try to set this to any of the other GPIOs that controls the LEDS to see this change, for instance set <NRF_PSEL(PWM_OUT0, 0, 14)>; to have the PWM output go to GPIO 14, i.e LED 2 instead of GPIO 13, i.e LED 1.

If you’re using a motor, we instead want to use a free available GPIO on the DK. This means that if you wish to use anything other than what we’ve described here, you need to consult with the product specification of the device you’re using and check if that GPIO is available for use. For the nRF52840DK you can find the product specification here: https://infocenter.nordicsemi.com/topic/ug_nrf52840_dk/UG/dk/intro.html .

I will use pin 3 in this exercise so this is what the code under step 1.4 in nrf52840dk_nrf52840.overlay should look like 

&pinctrl {
    pwm0_custom: pwm0_custom {
        group1 {
            psels = <NRF_PSEL(PWM_OUT0, 0, 3)>;
            nordic,invert;
        };
    };

    pwm0_csleep: pwm0_csleep {
        group1 {
            psels = <NRF_PSEL(PWM_OUT0, 0, 3)>;
            low-power-enable;
        };
    };
};

Step 2 - Motor control

We have now set up our device to have a PWM output on pin 3, but we still need to decide how to set the pin. In this part we will create a basic function that sets the motor angle based on an input parameter.

This input parameter will be decided based on the datasheet of the motor. For the SG90 servo motor you can see that here: http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf 

2.1 - Create a function to set the motor angle

In main.c there is defined a function called set_motor_angle(). This function should return an error code, which is 0 on success and negative on error.

Give this function the PWM duty cyle or an angle between 0 and 180 degrees as an input parameter

int set_motor_angle(uint32_t duty_cycle_ns)

2.2 - Define a maximum and minimum duty cycle

Inspect the datasheet of the motor and define these parameters based on this. Do note that you may need to do some experimentation as what corresponds to minimum and maximum angle may vary from motor to motor due to wear and tear. I set the following as my parameters:

#define PWM_MIN_DUTY_CYCLE 600000
#define PWM_MAX_DUTY_CYCLE 2000000

2.3 - Test your code

Connect the wiring on the motor. Connect the motors ground to ground, the VCC to a voltage source on the DK and the PWM wire to the GPIO you set in step 1.4.

This is how the wiring looks if you’ve used the GPIO on port 0, pin 3



Image 4: Setup

Inside the loop in main.c call set_motor_angle with different duty cycle. Add the following to “step 2.3” in main() 

        LOG_INF("Setting motor angle to %d",PWM_MIN_DUTY_CYCLE);
        set_motor_angle(PWM_MIN_DUTY_CYCLE);
        k_sleep(K_SECONDS(5));
        LOG_INF("Setting motor angle to %d",PWM_MAX_DUTY_CYCLE);
        set_motor_angle(PWM_MAX_DUTY_CYCLE);
        k_sleep(K_SECONDS(5));

This is not the greatest way to set the motor angle as this will just continue to change the angle at an interval set in the sleep. Do also note that you may have to tweak the duty cycles based on your motors parameters, and in case you’ve done this exercise only using the LED you might also need to do some tweaking.

Instead of simply changing the angle in a loop, we want to use the DKs buttons to set which motor angle that has been used.

Step 3 - Add button control to your servo motor application

3.1 - Enable the buttons in prj.conf

In prj.conf add to step 3.1

CONFIG_DK_LIBRARY=y

3.2 - Create a configure_dk_buttons_and_leds function in main.c

This function should initialize the buttons using dk_buttons_init(). dk_buttons_init requires an input parameter, in this case a callback function. We will define this callback function in step 3.3 and name it button_handler. 

Under step 3.2 in main.c add the following:

static void configure_dk_buttons_and_leds(void)
{
    int err;

    err = <Add your function call to dk_buttons_init() here>
    if (err) {
        LOG_ERR("Couldn't init buttons (err %d)", err);
    }
}

Note that this is named “_and leds”. We will get back to the LED part in step 4 in this exercize.

3.3 - Initialize the buttons 

Add a function call to configure_dk_buttons_and_leds() in main() under step 3.2

3.4 - Define a button handler callback function called button_handler

This callback function should check for error codes and switch between different cases based on what button on the DK has been pressed. For now use button 1 and button 2 to set the motor angle by using the minimum and maximum duty cycle you’ve defined.

We will use some items from dk_buttons_and_leds.h in our callback function. The first item is to use the button masks, i.e DK_BTN1_MSK and DK_BTN2_MSK for our cases. We will also use the masks button_state and has_changed to get the state of the button pressed and to know if said button has been pressed i.e has changed.



Under step 3.3 in main.c add the following:

void button_handler(uint32_t button_state, uint32_t has_changed)
{
    int err = 0;
	int button_pressed = 0;
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
			case DK_BTN1_MSK:
			err = <Add your code to set motor angle based on minimum duty cycle>
			LOG_INF("Setting motor angle to %d",PWM_MIN_DUTY_CYCLE);

			case DK_BTN2_MSK:
            err = <Add your code to set motor angle based on maximum duty cycle>
            LOG_INF("Setting motor angle to %d",PWM_MAX_DUTY_CYCLE);
				break;
			default:
				break;
		}
        LOG_INF("Button %d pressed.", button_pressed);
        if (err) {
            LOG_ERR("couldn't set duty cycle. Err %d", err);
        }
		LOG_INF("Button %d pressed.", button_pressed);
	}
}

Add your own function call to the code you wrote in step 3.2 to set the motor angle in the two cases.

This is just one way to create the desired callback function and if you have another way to do so which you prefer, then feel free to use that implementation instead.

3.5 - Test your code

Replace your main() with the following, build and flash your firmware.

void main(void)
{
        LOG_INF("Starting PWM basic sample \n");
        /* Step 3.2 - Call the initialization function */
        motor_init();
        configure_dk_buttons_and_leds();

        for (;;){
        k_sleep(K_SECONDS(1));
        }
             
}

If you’ve done everything correct you should now be able to change the motors angle by pressing button 1 or button 2. 

If you’ve followed along this exercise without the motor and instead configured everything for the PWM LED from exercise 1 you should now be able to make the LED blink with two different frequencies. Do note that the parameters chosen for the motor control may not work for a LED so some tweaking might be needed.

Step 4 - Multiple PWM instances

We now want to expand on this exercise by adding another pwm instance. If you’ve used a motor for the previous steps, we can take a step back 

TODO 16.11 - Clean and test code. Need access to the motor in the office
