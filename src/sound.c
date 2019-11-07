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

void init_wavetable();
void init_wavetable()
{
  int x;
  for(x=0; x<N; x++)
    wavetable[x] = 32767 * sin(2 * M_PI * x / N);
}


void setup_dac();
void setup_dac() {
    /* Student code goes here */
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	DAC->CR &= ~DAC_CR_EN1;
	DAC->CR &= ~DAC_CR_BOFF1;
	DAC->CR |= DAC_CR_TEN1; //trigger enable
	DAC->CR |= DAC_CR_TSEL1; //set software trigger
	DAC->CR |= DAC_CR_EN1; //enable

}
void TIM6_DAC_IRQHandler() {
    /* Student code goes here */
	DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1; //trigger the conversion
	TIM6->SR &= ~TIM_SR_UIF; //ack interrupt




	//step 1 code - 6.3
	/*
	offset += step;
	if((offset>>16) >= N)//if past end of array
	{
		offset -= N<<16;
	}
	int debug_val = offset>>16;
	int sample = wavetable[offset>>16];
	sample = sample /16 +2048;
	DAC->DHR12R1 = sample;
	*/



	//step 2 code - 6.4
	/*
	offset1 += stepC;
	if((offset1>>16) >= N)//if past end of array
	{
		offset1 -= N<<16;
	}
	offset2 += stepF;
	if((offset2>>16) >= N)//if past end of array
	{
		offset2 -= N<<16;
	}
	int sample = 0;
	sample += wavetable[offset1>>16];
	sample += wavetable[offset2>>16];

	sample = sample /32 +2048;
	if(sample > 4095) sample = 0; //clip
	DAC->DHR12R1 = sample;
	*/


	//step 3 code - 6.5

	offset1 += step0;
	if((offset1>>16) >= N)//if past end of array
	{
		offset1 -= N<<16;
	}
	offset2 += step1;
	if((offset2>>16) >= N)//if past end of array
	{
		offset2 -= N<<16;
	}
	int sample = 0;
	sample += wavetable[offset1>>16];
	sample += wavetable[offset2>>16];

	sample = sample /32 +2048;
	if(sample > 4095) sample = 0; //clip
	else if(sample < 0) sample =0; //clip
	DAC->DHR12R1 = sample;


}
