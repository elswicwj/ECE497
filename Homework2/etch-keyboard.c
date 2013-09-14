// Written by Will Elswick on 9/13/2013
//  Lets the user draw on an etch a sketch displayed on the console
//   using the WASD keys.
//
//  As a bonus, I've added that the c key clears the screen (saving
//  your position) and r toggles drawing/erasing.

#include "etch-keyboard.h"

void signal_handler(int sig) {
	printf("Ctrl-C pressed, cleaning up and exiting...\n");
	keepgoing = 0;
}

//Actually makes the movements
void move(int* pos, char dir, int max_x, int max_y) {
	int oldx = pos[0];
	int oldy = pos[1];
	switch(dir) {
		case 'w': pos[1]--; break;
		case 's': pos[1]++; break;
		case 'a': pos[0]--; break;
		case 'd': pos[0]++; break;
	}
	if((pos[0] == max_x) || (pos[0] == -1)) pos[0] = oldx;
	if((pos[1] == max_y) || (pos[1] == -1)) pos[1] = oldy;
}

//Draws the array it's given
void draw_board(char** board, int size_x, int size_y) {
	int i, j;
	system("clear");
	for(j = 0; j < size_y; j++) {
		char line[size_x + 2];
		for(i = 0; i < size_x; i++) {
			line[i] = board[i][j];
		}
		line[i++] = '\n';
		line[i] = '\0';
		printf(line);
	}
}

//Clears the board
void clear_board(char** board, int size_x, int size_y) {
	int i;
	for(i = 0; i < size_x; i++) {
		memset(board[i], '_', size_y * sizeof(char));
	}
}

int main(int argc, char **argv, char **envp) {

	keepgoing = 1;
	char cursor_char = 'X';
	
	if(argc != 5) {
		printf("Usage: %s <board width> <board height> <start x> <start y>\n", argv[0]);
		exit(-1);
	}
	
	//Set up board
	int i, j;
	int size_x = atoi(argv[1]);
	int size_y = atoi(argv[2]);
	char** board;
	//Malloc memory for the board
	board = (char**) malloc(size_x * sizeof(char*));
	for(i = 0; i < size_x; i++) {
		board[i] = (char*) malloc(size_y * sizeof(char));
		memset(board[i], '_', size_y * sizeof(char));
	}
	int pos[2] = {atoi(argv[3]), atoi(argv[4])};
	
	//Wait for inputs and draw after each input. This scanf can later be
	// replaced with poll() when we want input from the buttons.
	board[pos[0]][pos[1]] = 'X';
	draw_board(board, size_x, size_y);
	while(keepgoing) {
		char dir;
		scanf("%c", &dir);
		if(dir == 'c') {
			clear_board(board, size_x, size_y);
			continue;
		}
		if(dir == 'r') {
			cursor_char = (cursor_char == 'X') ? '_': 'X';
			continue;
		}
		board[pos[0]][pos[1]] = cursor_char;
		move(pos, dir, size_x, size_y);
		draw_board(board, size_x, size_y);
	}

	//Free up the memory we used
	for(i = 0; i < size_x; i++) {
		free(board[i]);
	}
	free(board);
	return 0;
}
