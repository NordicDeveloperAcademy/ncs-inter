#Exercise 3 - Control a motor with the PWM peripheral

In this excercize we will take a step back to excersize 1 so it is not a problem if you've not completed excersize 2.

We will use the MG90S metal gear servo motor for this excersize, but you can follow along without any additional hardware than the DK if you would like to.

The data sheet can be found here http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf

From the datasheet we can see that the PWM period is 20ms (50Hz) and the duty cycle is in between 1ms and 2ms. If you did excersize 1, you may remember that these values are the ones we started the excersize with when we tested the PWM peripheral for controlling LED1, and we observed that these values only resulted in a dimly lit LED. For the motor on the other hand, the values must be within this.

The servo can rotate approximately 90 degrees in each direction. 1ms pulse corresponds to -90 degrees, 1.5ms corresponds to 0 degrees and 2ms corresponds to 90 degrees rotation.

In lesson 1 we used LED1 as our PWM LED, which is connected to p0.13. We need to use one of the available GPIOs for our motor, and to do this we need to make some modifications to the device tree.

You can inspect the nRF52840dk_nrf52840.dts and see how it is configured here. Do note that we will not make our modifications here, and instead we will make the modifications in an overlay file which is specific for this project. To learn more about overlay files, see [THIS LESSON AND OR THESE LINKS]

[ADD IMAGE SHOWING THE OVERLAY]

Here you see pwm_leds

Here you can also see that PWM_POLARITY_INVERTED is set. For our motor control we wish to have a normal polarity so we will modify that as well.

The starting point in this excercize is the same as the solution in excersize 1.

## Step 1
### Step 1.1 - Create an overlay file
In this case, the overlay is already created. If you wish to do this on your own at a later point you can create one in your project folder and  name it "<your_board>.overlay". If you're using a nRF52840DK it will be "nrf52840dk_nrf52840.overlay".

### Step 1.2 - Add a pwm_led instance
Copy the pwm_led instance from nrf52840dk_nrf52840.dts / <your_board>.dts and paste it into your projects overlay file.

### Step 1.3 - Add the pwm0 instance 
Right click &pwm0 in the instance of pwm_leds copied in and click "go to definition" in the device tree. 

Observe that the pin numbers are set in pmw0_default and pwm0_sleep. Now copy the &pinctrl instance into your overlay. 

Now, inspect pwm0_Default and pwm0_sleep by right_clicking and go to definition to see what these do and how they are defined

We will modify this in step 1.4 to pwm0_custom and pwm0_csleep. Note that you can call them other things than what we've chosen here.


### Step 1.4 - Add the instance for pwm0_custom and pwm0_csleep 
Inspect and copy pwm0_default and pwm0_sleep into the overlay file. Change the names to what you set it to in 1.3, i.e from _default and _sleep to _custom and _csleep
 

Copy the pwm_led instance from nrf52840dk_nrf52840.dts / <your_board>.dts and paste it into your projects overlay file.

## Step 2
### Step 2.1 - Create a function to set the motor angle

Define a function set_motor_angle() which should return an error code (0 on success, negative value on error).

Give the function a input parameter PWM duty cycle or an angle between 0 and 180 degrees

### Step 2.2 - Define a maximum and minimum duty cycle.
Do note that you may need to do some experimentation as what corresponds to minimum and maximum angle may vary from motor to motor. 

### Step 2.3 - Test your code 
To main() add a loop and call set_motor_angle() with different duty cycles

Do note that you may need to do some experimentation as what corresponds to minimum and maximum angle may vary from motor to motor. I choose 

Look into the datasheet of the motor and see what type of pins the coloured wires correspond to, and connect VCC to 5v, ground to ground and the PWM wire to the pin we defined in the overlay.

## Step 3 - Optional - add button features to change based on buttons
Add a button handler callback that changes the motor angle based on the buttons pressed. You will need to include CONFIG_DK=y, dk_buttons_and_leds.h, initialize the dk buttons and a callbackfunction that detects the button inputs.
