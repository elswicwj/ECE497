/* Created 9/12/2013 by Will Elswick for the Beagle Bone Black using gedit.
 *
 * This program set a high and low temperature threshold on 2 temperature
 * sensors and then notifies the console when the sensors trip.
 */

#include "control_tempsensors.h"

void signal_handler(int sig) {
	keepgoing = 0;
	printf("\nCtrl-C pressed--exiting...\n");
}

int main(int argc, char **argv, char **envp)
{
	signal(SIGINT, signal_handler);

	if(argc != 5) {
		printf("Usage: %s <sensor 1 low threshold> <sensor 1 high threshold> <sensor 2 low threshold> <sensor 2 high threshold>", argv[0]);
		exit(-1);
	}

	int i;	

	int sensor_addresses[] = SENSOR_ADDRESSES;
	int alerts[] = ALERT_PINS;
	int alert_size = sizeof(alerts)/sizeof(alerts[0]);

	struct pollfd fdset[alert_size];
	int nfds = alert_size;	

	int gpio_fd[alert_size];
	
	int register_counter = 1;
	for(i = 0; i < alert_size; i++)
	{
		gpio_export(alerts[i]);
		gpio_set_dir(alerts[i], "in");
		gpio_set_edge(alerts[i], "falling");
		gpio_fd[i] = gpio_fd_open(alerts[i], O_RDONLY);
		
		char set_low[100];
		sprintf(set_low, "i2cset -y %d %d %d %s b", BUS, sensor_addresses[i], THRESHOLD_LOW_REGISTER, argv[register_counter++]);
		system(set_low);
		
		char set_high[100];
		sprintf(set_high, "i2cset -y %d %d %d %s b", BUS, sensor_addresses[i], THRESHOLD_HIGH_REGISTER, argv[register_counter++]);
		system(set_high);
	}

	while(keepgoing)
	{
		memset((void*)fdset, 0, sizeof(fdset));
		for(i = 0; i < alert_size; i++)
		{		
			fdset[i].fd = gpio_fd[i];
			fdset[i].events = POLLPRI;
			char buf[1];
			read(fdset[i].fd, buf, 1);
		}
		poll(fdset, nfds, TIMEOUT);
		for(i = 0; i < alert_size; i++)
		{
			if(fdset[i].revents & POLLPRI) 
			{
				printf("Sensor %d has tripped!\n", i);
				char read_command[100];
				sprintf(read_command, "./tempread.sh %d %d", BUS, sensor_addresses[i]);
				system(read_command);
			}
		}
		fflush(stdout);
	}

	for(i = 0; i < alert_size; i++)
	{
		gpio_fd_close(gpio_fd[i]);
	}
	return 0;
}
