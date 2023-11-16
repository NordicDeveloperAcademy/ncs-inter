# Pulse Width Modulation
Pulse Width Modulation (PWM) is a useful technique to use digital control to create square waves, a signal switching from on and off, to control for instance a LED or a motor. The square wave generated can only be high or low at a given time, and by controlling the signals frequency and in turn the period of the signal, we can use this to set the "duty cycle" of the signal.

 The PWM module works as a counter counting from 0 up to a PWM period. It starts with the PWM pin being high, and when it reaches a certain value called the PWM duty cycle the PWM signal will go low. When the counter reaches PWM period, the PWM pin will reset to high. See the figure from our servo motor specification:

A duty cycle is used to define the relative time in percentage that the PWM signal is high versus when it is low over a time period. For instance, a 50% duty cycle spends equal amounts of time as a high signal and low signal. 

Using this time ratio we can control the duty cycle to and use it to determine if one should increase or decrease the intensity of for instance a LED. 

In addition to the duty cycle, the frequency of the signal is important. 

# Goal of this sample
We want to output a PWM signal, and the duty cycle of the PWM signal is our control signal. We will first see how we enable the module, then modify our initial build to become a blinky sample controlled by the PWM and lastly we will see how we can use additional hardware in combination with the PWM module and a motor

In pwm_basic you will see how to control the pwm-led and set it to have a certain brightness. In pwm_motor, which requires additional hardware, we will see how we can use the PWM signal to control the position/angle of a motor

The first sample is largely inspired by the blinky_pwm sample found in nRF Connect SDK

# Links to documentations used in this repo
* https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/build/dts/api/api.html#pwm
* https://www.electronicoscaldas.com/datasheet/MG90S_Tower-Pro.pdf

# Other resources
* For how servo motors work: https://www.jameco.com/Jameco/workshop/Howitworks/how-servo-motors-work.html 


