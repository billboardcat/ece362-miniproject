/*
 * sound.h
 *
 *  Created on: Nov 7, 2019
 *      Author: pacmanpark
 */

#ifndef SOUND_H_
#define SOUND_H_

void init_wavetable();
void setup_dac_gpio();
void init_wavetable();
void setup_timer6();
void setup_dac();
void set_note(char note);
void play_note(char note);
void stop_note();
void init_beep();
void nano_wait(unsigned int n);

#endif /* SOUND_H_ */
