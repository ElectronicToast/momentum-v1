/**
 * @file speaker.c
 * @brief Speaker driver
 */


#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/sync.h"

#include "speaker.h"
#include "config.h"
#include "pinmap.h"
#include "utilities.h"

// Select which tunes file to include
#ifdef TUNES_USE_EP4
    #include "tunes_ep4_44k1.h"
#elif defined(TUNES_USE_OBS)
    #include "tunes_obs_44k1.h"
#elif defined(TUNES_USE_CLASSIC)
    #include "tunes_classic_44k1.h"
#elif defined(TUNES_USE_OBS_CLASSICPWR)
    #include "tunes_obs_classicpower_44k1.h"
#else
    #include "tunes_obs_originalpower_44k1.h"
#endif


volatile bool play_repeat = false;
volatile bool done_playing = false;

volatile bool playing_poweron = false;

// The audio buffer
static const uint8_t* audio_buffer;
static uint32_t audio_buffer_size;

// The DMA sample data (4x audio samples = bytes) and its address
static uint32_t dma_sample = 0;
static uint32_t* dma_sample_addr = &dma_sample;

// 3 DMA channels
//  - PWM channel: writes the sample data to the PWM. Chained to stream channel.
//  - Trigger channel: triggers the PWM channel.
//  - Stream channel: copies 1 byte of audio sample to the fixed DMA sample address.
static int dma_pwm_chan;
static int dma_trig_chan;
static int dma_stream_chan;
static dma_channel_config dma_pwm_cfg;
static dma_channel_config dma_trig_cfg;
static dma_channel_config dma_stream_cfg;


void dma_irq_handler() {

    // To play once, just don't retrigger the trigger channel
    if (play_repeat) {
        dma_hw->abort = 1u << dma_trig_chan;
        dma_hw->ch[dma_stream_chan].al1_read_addr = (io_rw_32) audio_buffer;
        dma_hw->ch[dma_trig_chan].al3_read_addr_trig = (io_rw_32) &dma_sample_addr;
    }
    // If just finished playing power on, go right to hum
    else if (playing_poweron) {
        dma_hw->abort = 1u << dma_trig_chan;
        playing_poweron = false;

        audio_buffer = TUNE_HUM_DATA;
        audio_buffer_size = TUNE_HUM_LEN;
        play_repeat = true;
        dma_hw->ch[dma_trig_chan].transfer_count = SPK_N_REPETITIONS * audio_buffer_size;
        dma_hw->ch[dma_stream_chan].al1_read_addr = (io_rw_32) audio_buffer;
        dma_hw->ch[dma_trig_chan].al3_read_addr_trig = (io_rw_32) &dma_sample_addr;
    }
    else {
        spk_disable();
        done_playing = true;
    }

    // Acknowledge the interrupt
    dma_hw->ints0 = 1u << dma_trig_chan;
}


void spk_init() {
    playing_poweron = false;

    // Disable the speaker on startup
    gpio_init(PIN_SPK_EN);
    gpio_set_dir(PIN_SPK_EN, GPIO_OUT);
    
    spk_disable();

    // Get PWM slice and set up PWM
    gpio_set_function(PIN_SPK_PWM, GPIO_FUNC_PWM);
    pwm_set_gpio_level(PIN_SPK_PWM, 0);

    int spk_pwm_slice = pwm_gpio_to_slice_num(PIN_SPK_PWM);
    pwm_config pwm_cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&pwm_cfg, SPK_PWM_CLKDIV);
    // Since data is 8-bit, counter top should be 8-bit top
    pwm_config_set_wrap(&pwm_cfg, SPK_PWM_COUNT_TOP);
    pwm_init(spk_pwm_slice, &pwm_cfg, true);

    // Set up DMA for playing the turnon sound by default
    audio_buffer = TUNE_POWERON_DATA;
    audio_buffer_size = TUNE_POWERON_LEN;

    dma_pwm_chan = dma_claim_unused_channel(true);
    dma_trig_chan = dma_claim_unused_channel(true);
    dma_stream_chan = dma_claim_unused_channel(true);

    dma_pwm_cfg = dma_channel_get_default_config(dma_pwm_chan);
    // Transfer 16 bytes to repeat the sample on both upper and lower halves of CC
    channel_config_set_transfer_data_size(&dma_pwm_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_pwm_cfg, false);
    channel_config_set_write_increment(&dma_pwm_cfg, false);
    // Chain to stream DMA channel when done
    channel_config_set_chain_to(&dma_pwm_cfg, dma_stream_chan);
    // Transfer on PWM DREQ
    channel_config_set_dreq(&dma_pwm_cfg, DREQ_PWM_WRAP0 + spk_pwm_slice);

    dma_channel_configure(
        dma_pwm_chan,
        &dma_pwm_cfg,
        &pwm_hw->slice[spk_pwm_slice].cc,     // Write to PWM slice CC register
        &dma_sample,
        SPK_N_REPETITIONS,                          // Do N_REPETITIONS transfers
        false                                   // Do not start yet 
    );

    dma_trig_cfg = dma_channel_get_default_config(dma_trig_chan);
    channel_config_set_transfer_data_size(&dma_trig_cfg, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_trig_cfg, false);
    channel_config_set_write_increment(&dma_trig_cfg, false);
    // Transfer on PWM cycle end
    channel_config_set_dreq(&dma_trig_cfg, DREQ_PWM_WRAP0 + spk_pwm_slice);

    dma_channel_configure(
        dma_trig_chan,
        &dma_trig_cfg,
        // Write to PWM DMA channel read address trigger
        &dma_hw->ch[dma_pwm_chan].al3_read_addr_trig,
        // Read from our address
        &dma_sample_addr,
        // Trigger once for each repetition * number of audio bytes
        SPK_N_REPETITIONS * audio_buffer_size,
        false                                   // Do not start yet 
    );

    // Interrupt when trigger channel is done
    dma_channel_set_irq0_enabled(dma_trig_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_stream_cfg = dma_channel_get_default_config(dma_stream_chan);
    channel_config_set_transfer_data_size(&dma_stream_cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_stream_cfg, true);
    channel_config_set_write_increment(&dma_stream_cfg, false);

    dma_channel_configure(
        dma_stream_chan,
        &dma_stream_cfg,
        // Write to our sample address
        &dma_sample,
        // Read from the audio array
        audio_buffer,
        // Do one transfer per PWM completion
        1,
        false                                   // Do not start yet 
    );
}


void spk_play(bool repeat) {
    play_repeat = repeat;
    done_playing = false;

    // Stop whatever is currently playing
    dma_channel_abort(dma_trig_chan);

    // Reconfigure everything that needs to be reconfigured
    // with a new audio buffer
    dma_channel_configure(
        dma_trig_chan,
        &dma_trig_cfg,
        // Write to PWM DMA channel read address trigger
        &dma_hw->ch[dma_pwm_chan].al3_read_addr_trig,
        // Read from our address
        &dma_sample_addr,
        // Trigger once for each repetition * number of audio bytes
        SPK_N_REPETITIONS * audio_buffer_size,
        false                                   // Do not start yet 
    );

    dma_channel_configure(
        dma_stream_chan,
        &dma_stream_cfg,
        // Write to our sample address
        &dma_sample,
        // Read from the audio array
        audio_buffer,
        // Do one transfer per PWM completion
        1,
        false                                   // Do not start yet 
    );

    // Hit it
    dma_channel_start(dma_trig_chan);
    gpio_put(PIN_SPK_EN, 1);
}



inline void spk_play_turnon() {
    audio_buffer = TUNE_POWERON_DATA;
    audio_buffer_size = TUNE_POWERON_LEN;
    playing_poweron = true;
    spk_play(false);
}

inline void spk_play_turnoff() {
    audio_buffer = TUNE_POWEROFF_DATA;
    audio_buffer_size = TUNE_POWEROFF_LEN;
    spk_play(false);
}

inline void spk_play_hum_repeat() {
    audio_buffer = TUNE_HUM_DATA;
    audio_buffer_size = TUNE_HUM_LEN;
    spk_play(true);
}

inline void spk_play_clash() {
    // Pick a random clash sound out of the TUNE_CLASH_COUNT available
    // Since the ROSC is easiest to generate perfect square ranges, we'll
    // take the modulo, though it messes with uniformness
    uint8_t i = rand_powof2(8) % TUNES_CLASH_COUNT;
    audio_buffer = TUNES_CLASH_DATA[i];
    audio_buffer_size = TUNES_CLASH_LENS[i];
    spk_play(false);
}

inline void spk_play_swing() {
    uint8_t i = rand_powof2(8) % TUNES_SWING_COUNT;
    audio_buffer = TUNES_SWING_DATA[i];
    audio_buffer_size = TUNES_SWING_LENS[i];
    spk_play(false);
}


inline void spk_stop() {
    // Stop whatever is currently playing
    play_repeat = false;    // Needed to stop the irq_handler from retriggering
    dma_channel_abort(dma_trig_chan);
    done_playing = true;
    spk_disable();
}

inline void spk_enable() {
    gpio_put(PIN_SPK_EN, 1);
}

inline void spk_disable() {
    gpio_put(PIN_SPK_EN, 0);
}

inline bool spk_is_done_playing() {
    return done_playing;
}

inline void spk_wait_until_done_playing() {
    while (!done_playing) {
        tight_loop_contents();
    }
}
