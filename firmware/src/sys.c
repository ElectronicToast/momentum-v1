/**
 * @file sys.c
 * @brief System functions
 */


#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/xosc.h"
#include "hardware/rosc.h"
#include "hardware/regs/clocks.h"
#include "hardware/structs/syscfg.h"
#include "hardware/structs/xip_ctrl.h"

#include "config.h"
#include "sys.h"

#include "tick.h"
#include "ledstrip.h"
#include "button.h"
#include "speaker.h"
#include "imu.h"


inline void sys_init() {
    // To save more power, use ROSC instead of XOSC

    // Should really rewrite the initialization code, but ah well
    set_sys_clock_khz(SYS_CLK_FREQ_KHZ, true);

    // Turn off unused peripheral clocks
    clock_stop(clk_usb);
    clock_stop(clk_adc);
    clock_stop(clk_rtc);
    clock_stop(clk_peri);

    // Turn off ROSC
    //rosc_disable();

    // Set up board peripherals
    tick_init();
    ledstrip_init();
    btn_init();
    spk_init();
    
    imu_i2c_init();
    imu_reset();
    imu_configure_interrupt();

    // Put the IMU to sleep after setup
    #ifdef IMU_SLEEP
        imu_goto_sleep();
    #endif
}


inline void sys_go_dormant() {
    //sleep_run_from_rosc();

    // Reimplement sleep_run_from_rosc() so that RTC clock isn't started
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH,
                    0, // No aux mux
                    6.5 * MHZ,
                    6.5 * MHZ);

    // CLK SYS = CLK_REF
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                    0, // Using glitchless mux
                    6.5 * MHZ,
                    6.5 * MHZ);

    pll_deinit(pll_sys);
    pll_deinit(pll_usb);
    xosc_disable();

    // Dormant until rising edge on PIN_SYS_WAKEUP (button press/release)
    gpio_set_dormant_irq_enabled(PIN_SYS_WAKEUP, 
                                 IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS, 
                                 true);

    // Power down the SRAM banks. Can power down at leat the USB
    //syscfg_hw->mempowerdown = 0x40;
    //rosc_set_dormant();
    //syscfg_hw->mempowerdown = 0x00;

    // TODO: this might actually be working; measure the current draw vs above.
    // Disabling XIP cache seems to have no power savings
    //xip_ctrl_hw->ctrl = 0x0000000c;     // Power down XIP cache
    rosc_write(&rosc_hw->dormant, ROSC_DORMANT_VALUE_DORMANT);
    syscfg_hw->mempowerdown = 0x7f;
    while(!(rosc_hw->status & ROSC_STATUS_STABLE_BITS));
    syscfg_hw->mempowerdown = 0x00;
    //xip_ctrl_hw->ctrl = 0x00000003;     // Power up XIP cache

    // After wakeup, set up clocks
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);
    clocks_init();
    set_sys_clock_khz(SYS_CLK_FREQ_KHZ, true);
}
