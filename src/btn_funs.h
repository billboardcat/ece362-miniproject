#ifndef BTN_FUNS_H	// Prevent recursive inclusion of the header function
#define BTN_FUNS_H

void init_btns(void);
int get_btn_press(void);
int get_btn_release(void);
int get_btn_pressed(void);

void setup_tim3(void);
void TIM3_IRQHandler(void);


#endif
