// Written by Will Elswick on 9/13/2013
//  Lets the user draw on an etch a sketch displayed on the console
//   using the WASD keys.
//
//  As a bonus, I've added that the c key clears the screen (saving
//  your position) and r toggles drawing/erasing.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int keepgoing;

void signal_handler(int sig);

//Actually makes the movements
void move(int* pos, char dir, int max_x, int max_y);

//Draws the array it's given
void draw_board(char** board, int size_x, int size_y);

//Clears the board
void clear_board(char** board, int size_x, int size_y);
