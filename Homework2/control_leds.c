/* Created 9/12/2013 by Will Elswick for the Beagle Bone Black using gedit.
 *
 * This program uses interrupts on 4 buttons to control 4 LEDs.
 */

#include "control_leds.h"

void signal_handler(int sig) {
	keepgoing = 0;
}

int main(int argc, char **argv, char **envp)
{
	signal(SIGINT, signal_handler);

	int i;	

	int buttons[] = BUTTON_GPIO_PINS;
	int button_size = sizeof(buttons)/sizeof(buttons[0]);
	int button_active_edges[] = BUTTON_ACTIVE_EDGES;
	
	int leds[] = LED_GPIO_PINS;
	int led_size = sizeof(leds)/sizeof(leds[0]);

	struct pollfd fdset[button_size];
	int nfds = 4;	

	int gpio_fd[button_size + led_size];
	for(i = 0; i < button_size; i++)
	{
		gpio_export(buttons[i]);
		gpio_set_dir(buttons[i], "in");
		gpio_set_edge(buttons[i], "both");
		gpio_fd[i] = gpio_fd_open(buttons[i], O_RDONLY);
	}
	for(i = 0; i < led_size; i++)
	{
		gpio_export(leds[i]);
		gpio_set_dir(leds[i], "out");
		gpio_fd[i+4] = gpio_fd_open(leds[i], O_RDONLY);
	}

	while(keepgoing)
	{
		memset((void*)fdset, 0, sizeof(fdset));
		for(i = 0; i < button_size; i++)
		{		
			fdset[i].fd = gpio_fd[i];
			fdset[i].events = POLLPRI;
		}
		poll(fdset, nfds, TIMEOUT);
		for(i = 0; i < button_size; i++)
		{
			if(fdset[i].revents & POLLPRI) 
			{	
				char buf[1];
				read(fdset[i].fd, buf, 1);	

				int button_state;
				gpio_get_value(buttons[i], &button_state);
				int led_state = (button_active_edges[i] == button_state);
				gpio_set_value(leds[i], led_state);
				printf("\nButton (GPIO %d) changed state to %d.\n", 
					buttons[i], button_state);
				printf("\tLED (GPIO %d) changed state to %d as a result\n", 
					leds[i], led_state);
			}
		}
		fflush(stdout);
	}

	for(i = 0; i < button_size + led_size; i++)
	{
		gpio_fd_close(gpio_fd[i]);
	}
	printf("\nCtrl-C pressed--exiting...\n");
	return 0;
}
