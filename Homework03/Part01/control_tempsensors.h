/* Created 9/12/2013 by Will Elswick for the Beagle Bone Black using gedit.
 *
 * This program set a high and low temperature threshold on 2 temperature
 * sensors and then notifies the console when the sensors trip.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include "gpio-utils.h"

#define TIMEOUT -1
#define BUS 1
#define SENSOR_ADDRESSES {73, 74}
#define ALERT_PINS {30, 31}
#define THRESHOLD_LOW_REGISTER 2
#define THRESHOLD_HIGH_REGISTER 3
#define READ_REGISTER 0

int keepgoing = 1;

void signal_handler(int sig);
