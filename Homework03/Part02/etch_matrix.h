// Written by Will Elswick on 9/13/2013
//  Lets the user draw on an etch a sketch displayed on the console
//   using the buttons connected to the BBB's gpio pins.
//
//  As a bonus, I've added that the c key clears the screen (saving
//  your position) and r toggles drawing/erasing.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include "gpio-utils.h"
#include "i2c-dev.h"
#include "i2cbusses.h"

#define LED_X_DIM 8
#define LED_Y_DIM 8

#define TIMEOUT -1
#define BUTTON_GPIO_PINS {60, 50, 51, 15, 30, 31}
//What state should the GPIO pin be when the button is pressed? i.e.
// i.e. is there a pull up or a pull down resistor?
#define BUTTON_ACTIVE_EDGES {0, 1, 1, 0, 0, 0} 

int keepgoing;

void signal_handler(int sig);

//Actually makes the movements
void move(int* pos, char dir, int max_x, int max_y);

//Draws the array it's given
void draw_board(char** board, int size_x, int size_y);

//Clears the board
void clear_board(char** board, int size_x, int size_y);
