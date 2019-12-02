#include "btn_funs.h"
#include "sound.h"
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "led_funs.h"

int8_t history[5] = {0};
int col = 0;

/* Usage:
 * 1. void init_btns(void)
 * 		-- Initializes GPIOC pins 0-10
 * 		-- PC0 is sent to all buttons
 * 		-- PC1-5 is read through; when a button is pushed, the corresponding input goes high
 * 		-- PC6-10 are set to output
 * 2. void setup_tim3(void)
 * 		-- initializes TIMER3 to generate interrupts
 * 3. int get_btn_pressed(void)
 * 		-- returns the number of the button pressed (0-4)
 */

void init_btns(void) {
	// Enable the clock to port A
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	// Clear the MODER bits for PB0-PB5
	GPIOC->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2 |
					  GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5);

	// Set PB0 as output. PB1-PB5 as input
	GPIOC->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 |
					 GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0 | GPIO_MODER_MODER10_0);

	// Enable the common output
	GPIOC->ODR = 0x1;

	// Clear the PUPDR bits for PB1-5
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1 | GPIO_PUPDR_PUPDR2 |
					  GPIO_PUPDR_PUPDR3 | GPIO_PUPDR_PUPDR4 | GPIO_PUPDR_PUPDR5);

	// Enable a pull-down PB1-PB5
	GPIOC->PUPDR |= (GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR2_1 | GPIO_PUPDR_PUPDR3_1 |
					 GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1);
}

int get_btn_pressed(void) {
	int key = get_btn_press();
	set_led(key);
	switch (key) {
		case 0:
			play_note('C');
			break;
		case 1:
			play_note('D');
			break;
		case 2:
			play_note('E');
			break;
		case 3:
			play_note('F');
			break;
		case 4:
			play_note('G');
			break;
	}
	while (key != get_btn_release()) {}
	stop_note();
	clr_led(key);
	return key;
}

int get_btn_press(void) {
	while (1) {
		for (int i = 0; i < 5; i++) {
			if (history[i] == 1) {
				return i;
			}
		}
	}
}

int get_btn_release(void) {
	while (1) {
		for (int i = 0; i < 5; i++) {
			if (history[i] == -2) {
				return i;
			}
		}
	}
}

void setup_tim3(void) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = 48 - 1;
	TIM3->ARR = 1000 - 1;				// Set PSC and ARR for 1ms interrupt
	TIM3->DIER |= TIM_DIER_UIE;			// Enable the update interrupt
	TIM3->CR1 |= TIM_CR1_CEN;			// Enable the timer counter
	NVIC->ISER[0] = (1 << TIM3_IRQn);	// Enable the interrupt.
}

void TIM3_IRQHandler(void) {
	// Get the state of the ODR for GPIO
	uint16_t idr_state = GPIOC->IDR;

	history[0] = history[0] << 1;
	history[0] = history[0] | ((idr_state >> 1) & 0x1);

	history[1] = history[1] << 1;
	history[1] = history[1] | ((idr_state >> 2) & 0x1);

	history[2] = history[2] << 1;
	history[2] = history[2] | ((idr_state >> 3) & 0x1);

	history[3] = history[3] << 1;
	history[3] = history[3] | ((idr_state >> 4) & 0x1);

	history[4] = history[4] << 1;
	history[4] = history[4] | ((idr_state >> 5) & 0x1);

	// Acknowledge the interrupt
	TIM3->SR &= ~TIM_SR_UIF;
}
