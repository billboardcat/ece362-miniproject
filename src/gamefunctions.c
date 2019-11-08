#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdint.h>
#include <stdio.h>
#include "gamefunctions.h"

#define SPI_DELAY 1337

// Extern declarations of function pointers in main.c.
//=========================================================================
// An inline assembly language version of nano_wait.
//=========================================================================
void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

//=========================================================================
// Generic subroutine to configure the LCD screen.
//=========================================================================
void generic_lcd_startup(void) {
    nano_wait(100000000); // Give it 100ms to initialize
    cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
    cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
    cmd(0x01);  // clear entire display
    nano_wait(6200000); // clear takes 6.2ms to complete
    cmd(0x02);  // put the cursor in the home position
    cmd(0x06);  // 0000 01IS: set display to increment
}

//=========================================================================
// Bitbang versions of cmd(), data(), init_lcd(), display1(), display2().
//=========================================================================
void bitbang_sendbit(int b) {
    const int SCK = 1<<13;
    const int MOSI = 1<<15;
    // We do this slowly to make sure we don't exceed the
    // speed of the device.
    GPIOB->BRR = SCK;
    if (b)
        GPIOB->BSRR = MOSI;
    else
        GPIOB->BRR = MOSI;
    //GPIOB->BSRR = b ? MOSI : (MOSI << 16);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = SCK;
    nano_wait(SPI_DELAY);
}

//===========================================================================
// Send a byte.
//===========================================================================
void bitbang_sendbyte(int b) {
    int x;
    // Send the eight bits of a byte to the SPI channel.
    // Send the MSB first (big endian bits).
    for(x=8; x>0; x--) {
        bitbang_sendbit(b & 0x80);
        b <<= 1;
    }
}

//===========================================================================
// Send a 10-bit command sequence.
//===========================================================================
void bitbang_cmd(char b) {
    const int NSS = 1<<12;
    GPIOB->BRR = NSS; // NSS low
    nano_wait(SPI_DELAY);
    bitbang_sendbit(0); // RS = 0 for command.
    bitbang_sendbit(0); // R/W = 0 for write.
    bitbang_sendbyte(b);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = NSS; // set NSS back to high
    nano_wait(SPI_DELAY);
}

//===========================================================================
// Send a 10-bit data sequence.
//===========================================================================
void bitbang_data(char b) {
    const int NSS = 1<<12;
    GPIOB->BRR = NSS; // NSS low
    nano_wait(SPI_DELAY);
    bitbang_sendbit(1); // RS = 1 for data.
    bitbang_sendbit(0); // R/W = 0 for write.
    bitbang_sendbyte(b);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = NSS; // set NSS back to high
    nano_wait(SPI_DELAY);
}

//===========================================================================
// Configure the GPIO for bitbanging the LCD.
// Invoke the initialization sequence.
//===========================================================================
void bitbang_init_lcd(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->BSRR = 1<<12; // set NSS high
    GPIOB->BRR = (1<<13) + (1<<15); // set SCK and MOSI low
    // Now, configure pins for output.
    GPIOB->MODER &= ~(3<<(2*12));
    GPIOB->MODER |=  (1<<(2*12));
    GPIOB->MODER &= ~( (3<<(2*13)) | (3<<(2*15)) );
    GPIOB->MODER |=    (1<<(2*13)) | (1<<(2*15));

    generic_lcd_startup();
}

//===========================================================================
// Display a string on line 1.
//===========================================================================
void nondma_display1(const char *s) {
    // put the cursor on the beginning of the first line (offset 0).
    cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//===========================================================================
// Display a string on line 2.
//===========================================================================
void nondma_display2(const char *s) {
    // put the cursor on the beginning of the second line (offset 64).
    cmd(0x80 + 64);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x] != '\0')
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//GAME CODE

void congrats(int winner) {
    // Configure the function pointers
    cmd = bitbang_cmd;
    data = bitbang_data;
    display1 = nondma_display1;
    display2 = nondma_display2;
    // Initialize the display.
    bitbang_init_lcd();
    // Write text.
    const char *msg2 = "                Congratulations! :)             ";
    int offset = 0;
    if (winner == 1) { // player 1 won
    	while(1) {
    		display1("Player 1 Wins!");
    		display2(&msg2[offset]);
    		nano_wait(100000000);
    		offset += 1;
    		if (offset == 32)
    			offset = 0;
    	}
    }
    if (winner == 2) { // player 2 won
        	while(1) {
        		display1("Player 2 Wins!");
        		display2(&msg2[offset]);
        		nano_wait(100000000);
        		offset += 1;
        		if (offset == 32)
        			offset = 0;
        	}
    }
    if (winner == 3) { // it's a tie
            while(1) {
            	display1("It's a tie!");
            	display2(&msg2[offset]);
            	nano_wait(100000000);
            	offset += 1;
            	if (offset == 32)
            		offset = 0;
            }
    }

}


int round_cnt = 0;
int points[2] = {0};
int score_update = 0;
int turn = 1;

volatile int count = 0;
volatile int num_pressed = 0;

void round_counter() {
	cmd = spi_cmd;
	data = spi_data;
	display1 = circdma_display1;
	display2 = circdma_display2;
	// Initialize the display.
	dma_spi_init_lcd();
	turn = 0;
	round_cnt = round_cnt + 1;
	if (round_cnt == 1) {
		//welcome message and announce round 1
		display1("JUST SIMON");
		display2("ROUND 1");
		nano_wait(10000000000);
		enter_pattern(turn);
	}
	if (round_cnt == 2) {
		//announce round 2
		display2("ROUND 2");
		nano_wait(1000000000);
		enter_pattern(turn);
	}
	if (round_cnt == 3) {
		display2("ROUND 3");
		nano_wait(1000000000);
		enter_pattern(turn);
	}
	if (round_cnt == 4) {
		//say game over, wait, and then display congrats
		display1("GAME OVER!");
		nano_wait(1000000000);
		if (points[0] > points[1]) {
			//player 1 wins
			congrats(1);
		}
		if (points[0] < points[1]) {
			//player 2 wins
			congrats(2);
		}
		if (points[0] == points[1]) {
			//it is a tie
			congrats(3);
		}
	}

}

void play_pattern(int turn) {
	cmd = spi_cmd;
	data = spi_data;
	display1 = circdma_display1;
	display2 = circdma_display2;
	// Initialize the display.
	dma_spi_init_lcd();

	int counter = 0;
	//if turn is 0, then player 2 should be guessing
	//if turn is 1, then player 1 should be guessing
	if (turn == 1) {
		while(counter <= 10) { //while light pattern is playing
			display1("Player 1,");
			display2("Watch pattern!");
			nano_wait(100000000);
			counter++;
		}
	}
	else { //turn is 0, so player 2 is guessing
		while(counter <= 10) {
			display1("Player 2,");
			display2("Watch pattern!");
			nano_wait(100000000);
			counter++;
		}
	}

	count = 0;
	num_pressed = 0;
	trigger_count();

}


// switching turns to enter a pattern
void enter_pattern(int turn) {
    // Configure the function pointers.
    cmd = spi_cmd;
    data = spi_data;
    display1 = nondma_display1;
    display2 = nondma_display2;
    // Initialize the display using the SPI init function.
    spi_init_lcd();
    // Display something simple.
    int offset = 0;
    int buttons = 0;
    const char *msgp1 = "          Player 1, enter a pattern!            ";
    const char *msgp2 = "          Player 2, enter a pattern!            ";
    char btn_msg[20];

    while(buttons <= 50) {
    if (turn == 0) { //player 1's turn now to enter a pattern
    	display2(&msgp1[offset]);
    	nano_wait(100000000);
    	offset += 1;
    	if (offset == 32)
    		offset = 0;
    }
    else { //turn = 1, player 2's turn to enter a pattern
    	display2(&msgp2[offset]);
    	nano_wait(100000000);
    	offset += 1;
    	if (offset == 32)
    	    offset = 0;
    }
    sprintf(btn_msg, "# Entered: %d ", buttons);
    display1(btn_msg);
    buttons++; //add a conditional here for button press
    }

    play_pattern(turn);
}


void check_and_score(int pattern, int time, int turn) { //pattern (1 for right 0 for wrong), turn (0 for p1, 1 for p2)
	//for some reason, turn always seems to be 1 in this function.
	//turn 0 means score is added to player 2, since he or she is the guesser

	char score_message[20];
    // Configure the function pointers
    cmd = spi_cmd;
    data = spi_data;
    display1 = circdma_display1;
    display2 = circdma_display2;
    // Initialize the display.
    dma_spi_init_lcd();
    // Write text.
    int flash_ten = 0;
    int score = 0;
    int ind = 0;

    if (pattern == 1) {
    	score = score + 300 / time;
    }

    if (score_update == 1) { //need to update player 1's score.
    	points[0] = points[0] + score;
    	ind = 0;
    }
    else {
    	points[1] = points[1] + score;
    	ind = 1;
    }

    //swapping turns
    if (turn == 0) {
    	turn = 1;
    }
    else {
        turn = 0;
    }

    //points[turn] = points[turn] + score;

    int offset = 0;
    while (flash_ten < 10) {
        if ((offset / 2) & 1) {
        	if (pattern == 1)  //guessed correctly
        		display1("Correct!");
        	else  //guessed incorrectly
        		display1("Wrong!");
        }
        else {
            display1("");
            flash_ten++;
        }

        sprintf(score_message, "Total score: %d ", points[ind]);
        //sprintf(score_message, "score: %d %d ", points[0], points[1]);
        display2(score_message);

        nano_wait(100000000);
        offset += 1;
        if (offset == 32)
            offset = 0;
    }

    if (score_update == 1) { //just updated player 1's score after their guess.
    	score_update = 0;
    	round_counter();
    }

    else { //just updated player 2's score after their guess, waiting for player 2 to enter a pattern now.
    	score_update++;
    	enter_pattern(turn);
    }
}

char line1[16] = {"# Guessed :"};
char line2[16] = {"Timer :"};
int calls = 0;
int pattern_accuracy = 1;

void countdown() {
    char line[20];
    char new_line[20];
    calls++;
    if(calls == 1000) {
    	calls = 0;
		if(count >= 0) {
			count++;
		}
		num_pressed++; //put a condition here to check if button has been pressed
		sprintf(line, "Timer : %d ", count);
		display2(line);
		sprintf(new_line, "# Guessed : %d ", num_pressed);
		display1(new_line);
		if (num_pressed == 5) {
			check_and_score(pattern_accuracy, count, turn);
		}
    }
}


void trigger_count(void) {
    // Configure the function pointers.
    cmd = spi_cmd;
    data = spi_data;
    display1 = circdma_display1;
    display2 = circdma_display2;
    // Initialize timer 6.
    init_tim6();
    // Initialize the display.
    dma_spi_init_lcd();
    count = 0;
    num_pressed = 0;
    display1(line1);
    display2(line2);
}
