/* Created 9/12/2013 by Will Elswick for the Beagle Bone Black using gedit.
 *
 * This program uses interrupts on 4 buttons to control 4 LEDs.
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
#define BUTTON_GPIO_PINS {60, 50, 51, 15}
#define BUTTON_ACTIVE_EDGES {0, 1, 1, 0} //What state should the GPIO pin be in for the LED to be on?
#define LED_GPIO_PINS {30, 31, 48, 49}

int keepgoing = 1;

void signal_handler(int sig);
