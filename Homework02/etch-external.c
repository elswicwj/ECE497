// Written by Will Elswick on 9/13/2013
//  Lets the user draw on an etch a sketch displayed on the console
//   using the buttons connected to the BBB's gpio pins.
//
//  As a bonus, I've added that the c key clears the screen (saving
//  your position) and r toggles drawing/erasing.

#include "etch-external.h"

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
	for(j = -1; j < size_y; j++) {
		char line[size_x + 3];
		line[0] = '|';
		for(i = 0; i < size_x; i++) {
			if(j == -1) line[i + 1] = '_';
			else line[i + 1] = board[i][j];
		}
		line[++i] = '\n';
		line[++i] = '\0';
		printf(line);
	}
}

//Clears the board
void clear_board(char** board, int size_x, int size_y) {
	int i;
	for(i = 0; i < size_x; i++) {
		memset(board[i], ' ', size_y * sizeof(char));
	}
}

int main(int argc, char **argv, char **envp) {

	signal(SIGINT, signal_handler);

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
		memset(board[i], ' ', size_y * sizeof(char));
	}
	int pos[2] = {atoi(argv[3]), atoi(argv[4])};
	
	//Set up GPIO pins and their interrupts
	int buttons[] = BUTTON_GPIO_PINS;
	int button_size = sizeof(buttons)/sizeof(buttons[0]);
	int button_active_edges[] = BUTTON_ACTIVE_EDGES;

	struct pollfd fdset[button_size];
	int nfds = button_size;	

	int gpio_fd[button_size];
	for(i = 0; i < button_size; i++)
	{
		gpio_export(buttons[i]);
		gpio_set_dir(buttons[i], "in");
		gpio_set_edge(buttons[i], button_active_edges[i] ? "rising" : "falling");
		gpio_fd[i] = gpio_fd_open(buttons[i], O_RDONLY);
	}
	
	//Wait for inputs and draw after each input. This scanf can later be
	// replaced with poll() when we want input from the buttons.
	board[pos[0]][pos[1]] = 'O';
	draw_board(board, size_x, size_y);
	while(keepgoing) {
		char dir;
		//Reset GPIO interrupts
		memset((void*)fdset, 0, sizeof(fdset));
		for(i = 0; i < button_size; i++) {		
			fdset[i].fd = gpio_fd[i];
			fdset[i].events = POLLPRI;
		}
		
		//Poll interrupts (this replaced the scanf!)
		poll(fdset, nfds, TIMEOUT);
		
		usleep(2000); //Debounce the buttons
		
		//Translate the external button pressed into what
		// used to be keypresses--now we can use the same
		// old function
		for(i = 0; i < button_size; i++) {
			if(fdset[i].revents & POLLPRI) {
				char buf[1];
				read(fdset[i].fd, buf, 1);
				
				int button_state;
				gpio_get_value(buttons[i], &button_state);
				if(button_state == button_active_edges[i]) {	
					switch(i) {
						case 0: dir = 'w'; break;
						case 1: dir = 's'; break;
						case 2: dir = 'a'; break;
						case 3: dir = 'd'; break;
						case 4: dir = 'c'; break;
						case 5: dir = 'r'; break;
					}
					if(dir == 'c') {
						clear_board(board, size_x, size_y);
					}
					if(dir == 'r') {
						cursor_char = (cursor_char == 'X') ? ' ': 'X';
					}
					board[pos[0]][pos[1]] = cursor_char;
					move(pos, dir, size_x, size_y);
					board[pos[0]][pos[1]] = 'O';
					draw_board(board, size_x, size_y);
				}
			}
		}
	}

	//Close up GPIOs
	for(i = 0; i < button_size; i++)
	{
		gpio_fd_close(gpio_fd[i]);
	}

	//Free up the memory we used
	for(i = 0; i < size_x; i++) {
		free(board[i]);
	}
	free(board);
	return 0;
}
