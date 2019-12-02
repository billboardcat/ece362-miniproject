#include "btn_funs.h"
#include "ptrn_funs.h"
#include "led_funs.h"
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void get_ptrn10(int* ptrn) {
	for (int ind = 0; ind < 10; ind++) {
		ptrn[ind] = get_btn_pressed();
	}
}

void ret_ptrn(int* ptrn, int size) {
	for (int ind = 0; ind < size; ind++) {
		set_led(ptrn[ind]);
		nano_wait(5000000);
		clr_led(ptrn[ind]);
		nano_wait(5000000);
	}
}

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}
