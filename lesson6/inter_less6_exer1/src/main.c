#include <zephyr.h>
/* Step 2.1 - Include the relevant headers for pwm*/

/* Step 1.2 - Include logging and define log module */


/* Step 2.2 - define pwm_led0 using the pwm_dt_spec struct */

/* STEP 3.1 - Define the desired pwm period and pulse */


/* Step 3 - Create an initializing function to verify that the device is enabled*/ 
int pwm_init(void){
        int err = 0;
        LOG_INF("Initializing pwm");

        /* Step 3.2 - Check if the device is ready */


        /* Step 3.3 - Set period and pulse in nanoseconds and check for error*/

        return err
}

void main(void)
{
        LOG_INF("Starting PWM basic sample \n");
        /* Step 3.3 - Call the initialization function */


}
