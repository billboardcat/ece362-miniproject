#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RATE 100000
#define N 1000

short int wavetable[N];
int offset = 0;

int stepC = 554.365 * N / RATE * (1<<16);
int stepF = 698.456 * N / RATE * (1<<16);
int tone_step;

void init_wavetable();
void setup_dac_gpio();
void init_wavetable();
void setup_timer6();
void setup_dac();
void set_note(char note);

void init_wavetable()
{
  int x;
  for(x=0; x<N; x++)
    wavetable[x] = 32767 * sin(2 * M_PI * x / N);
}

// This function
// 1) enables clock to port A,
// 2) sets PA4 to analog mode 11
void setup_dac_gpio() {
    /* Student code goes here */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= 0x300;
}

// This function should enable the clock to the
// onboard DAC, enable trigger,
// setup software trigger and finally enable the DAC.
void setup_dac() {
    /* Student code goes here */
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	DAC->CR &= ~DAC_CR_EN1;
	DAC->CR &= ~DAC_CR_BOFF1;
	DAC->CR |= DAC_CR_TEN1; //trigger enable
	DAC->CR |= DAC_CR_TSEL1; //set software trigger
	DAC->CR |= DAC_CR_EN1; //enable

}
// This function should,
// enable clock to timer6,
// setup pre scalar and arr so that the interrupt is triggered 100us,
// enable the timer 6 interrupt and start the timer.
void setup_timer6() {
    /* Student code goes here */
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->ARR = 48-1;
	TIM6->PSC = 10-1;
	TIM6->DIER |= TIM_DIER_UIE;
	TIM6->CR1 |= TIM_CR1_CEN;

	NVIC->ISER[0] = 1<<TIM6_DAC_IRQn;
}
void TIM6_DAC_IRQHandler() {
    /* Student code goes here */
	DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1; //trigger the conversion
	TIM6->SR &= ~TIM_SR_UIF; //ack interrupt

	//step 1 code - 6.3

	offset += tone_step;
	if((offset>>16) >= N)//if past end of array
	{
		offset -= N<<16;
	}
	int debug_val = offset>>16;
	int sample = wavetable[offset>>16];
	sample = sample /16 +2048;
	DAC->DHR12R1 = sample;

}

//TODO need to fill with all possible note
//takes in char (capital letter) for note to play - switches global step variable to correct step freq.
void set_note(char note){
	switch(note){
	case 'C':
		tone_step = stepC;
		break;
	case 'F':
		tone_step = stepF;
		break;

	default:
		tone_step = stepC;
	}
}
