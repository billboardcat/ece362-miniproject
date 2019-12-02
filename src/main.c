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
#include "gamefunctions.h"

// These are function pointers.  They can be called like functions
// after you set them to point to other functions.
// e.g.  cmd = bitbang_cmd;
// They will be set by the stepX() subroutines to point to the new
// subroutines you write below.
void (*cmd)(char b) = 0;
void (*data)(char b) = 0;
void (*display1)(const char *) = 0;
void (*display2)(const char *) = 0;

// This array will be used with dma_display1() and dma_display2() to mix
// commands that set the cursor location at zero and 64 with characters.
//
uint16_t dispmem[34] = {
        0x080 + 0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x080 + 64,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};

//=========================================================================
// Subroutines for step 2.
//=========================================================================
void spi_cmd(char b) {
    // Your code goes here.
	// wait for the TXE bit to be clear
	// indicating that the SPI channel 2 transmitter bit is empty.
	// Then they should deposit the data in the
	// argument into the SPI channel 2 data register.

		while ((SPI2->SR & SPI_SR_TXE) == 0) {

		}
		SPI2->DR = b;

}

void spi_data(char b) {
    // Your code goes here.
	//add in an extra 0x200 to the data written so that it sets the
	//register select bit as described in Section 5.4.
	while ((SPI2->SR & SPI_SR_TXE) == 0) {
	}
	SPI2->DR = b + 0x200;


}

void spi_init_lcd(void) {
    // Your code goes here.

	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER |= 0x8a000000;
	GPIOB->AFR[1] &= ~(0xffffffff);

	SPI2->CR1 = SPI_CR1_BIDIMODE |SPI_CR1_BIDIOE | SPI_CR1_MSTR | SPI_CR1_BR;
	SPI2->CR1 &= ~(SPI_CR1_CPOL);
	SPI2->CR1 &= ~(SPI_CR1_CPHA);
	SPI2->CR2 = SPI_CR2_NSSP | SPI_CR2_SSOE | SPI_CR2_DS_3 | SPI_CR2_DS_0;
	SPI2->CR1 |= SPI_CR1_SPE;

	generic_lcd_startup();
}

// Display a string on line 1 using DMA.
void dma_display1(const char *s) {
    // Your code goes here.
    int x;
    for(x=0; x<16; x+=1) {
        if (s[x] != '\0') {
        	dispmem[x+1] = s[x] | 0x200;
        }
    }
    for(   ; x<34; x+=1) {
    	dispmem[x+1] = 0x220;
    }

    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel5->CMAR = (uint32_t) dispmem;
    DMA1_Channel5->CPAR = (uint32_t) (&(SPI2->DR));
    DMA1_Channel5->CNDTR = 17;
    DMA1_Channel5->CCR |= DMA_CCR_DIR;

    DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;

    DMA1_Channel5->CCR |= DMA_CCR_MINC;
    DMA1_Channel5->CCR &= ~DMA_CCR_PL;

    SPI2->CR2 |= SPI_CR2_TXDMAEN;
    DMA1_Channel5->CCR |= DMA_CCR_EN;

}

void dma_spi_init_lcd(void) {
    // Your code goes here.

	spi_init_lcd();
	//cmd(0x80 + 0);
	DMA1_Channel5->CCR &= ~(DMA_CCR_EN);
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    DMA1_Channel5->CMAR = (uint32_t) dispmem;
    DMA1_Channel5->CPAR = (uint32_t) (&(SPI2->DR));
    DMA1_Channel5->CNDTR = 34;
    DMA1_Channel5->CCR |= DMA_CCR_DIR;
    DMA1_Channel5->CCR |= DMA_CCR_CIRC;

    DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel5->CCR |= DMA_CCR_PSIZE_0;

    DMA1_Channel5->CCR |= DMA_CCR_MINC;
    DMA1_Channel5->CCR &= ~(DMA_CCR_PINC);
    DMA1_Channel5->CCR &= ~DMA_CCR_PL;

    SPI2->CR2 |= SPI_CR2_TXDMAEN;
    SPI2->SR |= SPI_SR_TXE;
    DMA1_Channel5->CCR |= DMA_CCR_EN;

}

// Display a string on line 1 by copying a string into the
// memory region circularly moved into the display by DMA.
void circdma_display1(const char *s) {
    // Your code goes here.
	int x;
	    for(x=0; x<16; x+=1) {
	        if (s[x]) {
	        	dispmem[x+1] = s[x] | 0x200;
	        }
	        else {
	        	break;
	        }
	    }
	    for(   ; x<16; x+=1) {
		dispmem[x+1] = 0x220;
	    }

}

//===========================================================================
// Display a string on line 2 by copying a string into the
// memory region circularly moved into the display by DMA.
void circdma_display2(const char *s) {
    // Your code goes here.
	int x;
	for(x=0; x<16; x+=1) {
       if (s[x]) {
        	dispmem[x+18] = s[x] | 0x200;
        }
       else {
          	break;
       }
    }
	for(   ; x<16; x+=1) {
	        dispmem[x + 18] = 0x220;
	}
}

void init_tim2(void) {
    // Your code goes here.
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->ARR = 100-1;
	TIM2->PSC = 480-1;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->CR1 |= TIM_CR1_CEN;
	NVIC->ISER[0] = 1 << TIM2_IRQn;
}

void init_tim15(void) {
	RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
	TIM15->ARR = 100-1;
	TIM15->PSC = 480-1;
	TIM15->DIER |= TIM_DIER_UIE;
	TIM15->CR1 |= TIM_CR1_CEN;
	NVIC->ISER[0] = 1 << TIM15_IRQn;
}

void TIM2_IRQHandler() {
	// Your code goes here.
	TIM2->SR &= ~TIM_SR_UIF;
	countdown();
}

//void TIM15_IRQHandler() {
//	TIM15->SR &= ~TIM_SR_UIF;
//	scrolling_msg();
//}

int main(void) {

	init_btns();
	setup_tim3();
	init_beep();

//	play_note('C');
//	nano_wait(1000000000);
//	int pattern[10] = {0};
//	get_ptrn10(pattern);
//	for (int i = 0; i < 5; i++) {
//		clr_led(i);
//	}
//	ret_ptrn(pattern, 10);
//	stop_note();

	round_counter();
}
