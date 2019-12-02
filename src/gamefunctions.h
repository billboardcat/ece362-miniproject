#ifndef GAMEFUNCTIONS_H
#define GAMEFUNCTIONS_H

void generic_lcd_startup(void);
void countdown();
void congrats(int winner);
void step2(void);
void enter_pattern();
void check_and_score(int pattern, int time);
void step6(void);
void play_pattern();
void round_counter(void);

extern void (*cmd)(char b);
extern void (*data)(char b);
extern void (*display1)(const char *);
extern void (*display2)(const char *);

void spi_cmd(char);
void spi_data(char);

//void spi_display1(const char *);
//void spi_display2(const char *);
void dma_display1(const char *);
void circdma_display1(const char *);
void circdma_display2(const char *);

void spi_init_lcd(void);
void dma_spi_init_lcd(void);

//void init_tim6(void);

void scrolling_msg();

#endif
