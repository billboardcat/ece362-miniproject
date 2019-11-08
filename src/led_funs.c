#include "btn_funs.h"
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void set_led(int led_num) {
	GPIOC->ODR |= (0x1 << (led_num + 6));
}

void clr_led(int led_num) {
	GPIOC->ODR &= ~(0x1 << (led_num + 6));
}
