/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "btn_funs.h"
#include "led_funs.h"
#include "ptrn_funs.h"
#include "sound.h"

int main(void) {

	init_btns();
	setup_tim3();
	init_beep();

//	play_note('C');
//	nano_wait(1000000000);
	int pattern[10] = {0};
	get_ptrn10(pattern);
	for (int i = 0; i < 5; i++) {
		clr_led(i);
	}
	ret_ptrn(pattern, 10);
//	stop_note();

}
