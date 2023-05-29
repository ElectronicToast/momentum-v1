/**
 * @file speaker.h
 * @brief Speaker driver
 */


#ifndef SPEAKER_H
#define SPEAKER_H


void spk_init();
void spk_play(bool repeat);

void spk_play_turnon();
void spk_play_turnoff();
void spk_play_hum_repeat();
void spk_play_clash();
void spk_play_swing();

void spk_stop();
void spk_enable();
void spk_disable();

bool spk_is_done_playing();
void spk_wait_until_done_playing();


#endif /* SPEAKER_H */
