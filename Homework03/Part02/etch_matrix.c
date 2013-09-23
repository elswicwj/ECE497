// Written by Will Elswick on 9/13/2013
//  Lets the user draw on an etch a sketch displayed on the console
//   using the buttons connected to the BBB's gpio pins.
//
//  As a bonus, I've added that the c key clears the screen (saving
//  your position) and r toggles drawing/erasing.

#include "etch_matrix.h"

static void help(void) __attribute__ ((noreturn));

static void help(void) {
	fprintf(stderr, "Usage: matrixLEDi2c (hardwired to bus 3, address 0x70)\n");
	exit(1);
}

static int check_funcs(int file) {
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		return -1;
	}

	if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
		fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
		return -1;
	}
	return 0;
}

// Writes block of data to the display
static int write_block(int file, __u16 *data) {
	int res;
#ifdef BICOLOR
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)data);
	return res;
#else
/*
 * For some reason the single color display is rotated one column, 
 * so pre-unrotate the data.
 */
	int i;
	__u16 block[I2C_SMBUS_BLOCK_MAX];
//	printf("rotating\n");
	for(i=0; i<8; i++) {
		block[i] = (data[i]&0xfe) >> 1 | 
			   (data[i]&0x01) << 7;
	}
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16, 
		(__u8 *)block);
	return res;
#endif
}

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
void draw_board(int file, char** board, int size_x, int size_y) {
	int i, j;
	__u16 led_board[]=
	{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
	system("clear");
	for(j = -1; j < size_y; j++) {
		char line[size_x + 3];
		line[0] = '|';
		for(i = 0; i < size_x; i++) {
			if(j == -1) line[i + 1] = '_';
			else {
				line[i + 1] = board[i][j];
				if(line[i + 1] == 'O') led_board[j]|=(0x0001<<(i+8));
				else if(line[i + 1] == 'X') led_board[j]|=(0x0001<<i);
			}
		}
		line[++i] = '\n';
		line[++i] = '\0';
		printf(line);
	}
	write_block(file, led_board);
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
		printf("Usage: %s <start x> <start y> <temp threshold low> <temp threshold high>\n", argv[0]);
		exit(-1);
	}
	
	//Set up LED matrix
	int res, i2cbus, address, file;
	char filename[20];
	int force = 0;

	i2cbus = lookup_i2c_bus("1");
	printf("i2cbus = %d\n", i2cbus);
	if (i2cbus < 0)
		help();

	address = parse_i2c_address("0x70");
	printf("address = 0x%2x\n", address);
	if (address < 0)
		help();

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
//	printf("file = %d\n", file);
	if (file < 0
	 || check_funcs(file)
	 || set_slave_addr(file, address, force))
		exit(1);
		
	//Set up temp sensor
	int temp_file;
	address = parse_i2c_address("0x4a");
	printf("address = 0x%2x\n", address);
	if (address < 0)
		help();

	temp_file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
//	printf("file = %d\n", file);
	if (temp_file < 0
	 || check_funcs(temp_file)
	 || set_slave_addr(temp_file, address, force))
		exit(1);
		
	// Check the return value on these if there is trouble
	i2c_smbus_write_byte(file, 0x21); // Start oscillator (p10)
	i2c_smbus_write_byte(file, 0x81); // Disp on, blink off (p11)
	i2c_smbus_write_byte(file, 0xe7); // Full brightness (page 15)
	
	//Set up board
	int i, j;
	int size_x = LED_X_DIM;
	int size_y = LED_Y_DIM;
	char** board;
	//Malloc memory for the board
	board = (char**) malloc(size_x * sizeof(char*));
	for(i = 0; i < size_x; i++) {
		board[i] = (char*) malloc(size_y * sizeof(char));
		memset(board[i], ' ', size_y * sizeof(char));
	}
	int pos[2] = {atoi(argv[1]), atoi(argv[2])};
	
	//Set up GPIO pins and their interrupts
	int buttons[] = BUTTON_GPIO_PINS;
	int button_size = sizeof(buttons)/sizeof(buttons[0]);
	int button_active_edges[] = BUTTON_ACTIVE_EDGES;
	int sensor_addresses[] = SENSOR_ADDRESSES;

	struct pollfd fdset[button_size];
	int nfds = button_size;	

	int gpio_fd[button_size];
	for(i = 0; i < button_size; i++)
	{
		gpio_export(buttons[i]);
		gpio_set_dir(buttons[i], "in");
		gpio_set_edge(buttons[i], button_active_edges[i] ? "rising" : "falling");
		gpio_fd[i] = gpio_fd_open(buttons[i], O_RDONLY);
		
		if( i > button_size - 3) {
			char set_low[100];
			sprintf(set_low, "i2cset -y %d %d %d %s b", BUS, sensor_addresses[i-4], THRESHOLD_LOW_REGISTER, argv[3]);
			system(set_low);

			char set_high[100];
			sprintf(set_high, "i2cset -y %d %d %d %s b", BUS, sensor_addresses[i-4], THRESHOLD_HIGH_REGISTER, argv[4]);
			system(set_high);
		}
	}
	
	//Wait for inputs and draw after each input. This scanf can later be
	// replaced with poll() when we want input from the buttons.
	board[pos[0]][pos[1]] = 'O';
	draw_board(file, board, size_x, size_y);
	res = i2c_smbus_write_byte(file, 0xe0);
	while(keepgoing) {
		char dir;
		//Reset GPIO interrupts
		memset((void*)fdset, 0, sizeof(fdset));
		for(i = 0; i < button_size; i++) {		
			fdset[i].fd = gpio_fd[i];
			fdset[i].events = POLLPRI;
		}
		
		//Poll interrupts (this replaced the scanf!)
		poll(fdset, nfds, 100);
		
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
					draw_board(file, board, size_x, size_y);
				}
			}
		}
	
		int temp = i2c_smbus_read_byte_data(temp_file, 0);
		//printf("%d", temp);
	  res = i2c_smbus_write_byte(file, 0xe0 - atoi(argv[3]) + temp);
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
