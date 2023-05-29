/**
 * @file main.c
 */

#include "sys.h"
#include "ledstrip.h"
#include "button.h"
#include "speaker.h"
#include "imu.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "config.h"

#include "hardware/rosc.h"


void setup_turnon() {
    // Uncomment if need to printf()
    //stdio_init_all();

    //gpio_init(PICO_DEFAULT_LED_PIN);
    //gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    //gpio_put(PICO_DEFAULT_LED_PIN, 0);

    ledstrip_turn_on();

    // Hum sound starts playing automatically after power-on
    spk_play_turnon();

    #ifdef IMU_RESET_ON_EVENT
        // DO NOT USE
        imu_i2c_init();
        imu_reset();
        imu_configure_interrupt();
    #else
        // IMU needs to be stationary for some time before motion is detected
        #ifdef IMU_SLEEP
            imu_wake_up();
        #endif
    #endif

    // Kludge to stop motion detection / button presses during poweron tune.
    // Since poweron goes to hum immediately on DMA interrupt, events during
    // poweron are only processed after the first hum sound has played all the
    // way through.
    // Should be longer than poweron tune length
    sleep_ms(SYS_POWERON_DELAY);

    btn_clear_press();
    imu_clear_motion();
}


int main()
{
    // Sleep until first button press
    sys_init();
    sys_go_dormant();

    // Code resumes here after any button pressB

    // Pick random color here for possibly better randomness versus using the 
    // ROSC random bit function on startup
    #ifdef SABER_STARTUP_COLOR_RANDOM
        ledstrip_set_random_color();
    #endif
    setup_turnon();

    while (true) {
        if (imu_has_clash()) {
            spk_stop();
            spk_play_clash();
            #ifdef LEDSTRIP_FLASH_ON_CLASH
                ledstrip_flash();
            #endif
        }
        if (imu_has_swing()) {
            spk_stop();
            spk_play_swing();
        }

        // After a motion, resume playing hum sound repeatedly
        if (spk_is_done_playing()) {
            spk_play_hum_repeat();
        }

        // Long press and release - change the LED strip color
        if (btn_has_long_press()) {
            ledstrip_next_color();
            
            #ifdef IMU_RESET_ON_EVENT
                imu_i2c_init();
                imu_reset();
                imu_configure_interrupt();
            #endif
        }

        // Short press - turn off
        if (btn_has_short_press()) {
            spk_stop();
            spk_play_turnoff();
            ledstrip_turn_off();

            spk_wait_until_done_playing();
            spk_disable();
            #ifdef IMU_SLEEP
                imu_goto_sleep();
            #endif

            sys_go_dormant();
            
            // Code resumes here after any button press
            setup_turnon();
        }
    }
}
