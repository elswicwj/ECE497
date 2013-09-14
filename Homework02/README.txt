Homework 2.
Created by Will Elswick 9/13/2013 using gedit.

In this repository are the 3 programs I have written for homework 1.
Use the 'make' command to compile all three executables simultaneously

control_leds
Usage: control_leds
This program allows you to control the output state of 4 GPIO pins
using 4 other GPIO pins. These are configurable in the #define
section at the top of the program. Because of the error with the
pin-outs shown on the beaglebone's webserver, I used pins other
than those mentioned in the handout for my testing. By altering the
GPIO numbers in the 3 defines you can change the GPIO pins numbers
for the buttons, wether they have a pull-up (0) resistor or pull-down
(1) resistor, and the GPIO pins for the LEDs, respectively.

etch-keyboard
Usage: etch-keyboard <board width> <board height> <start x> <start y>
This is the basic etch-a-sketch program with takes input from the 
keyboard and displays it in the console. To control the cursor, 
use the 'w', 'a', 's', 'd' keys for up, left, down, and right respectively.
Press return after each key to submit your move. As an extra,
submitting the 'c' key clears the display and the 'r' key toggles 
erase mode.

etch-external
Usage: etch-external <board width> <board height> <start x> <start y>
This program is a combination of the two above programs. The cursor
is now controlled by the four external buttons listed in the defines.
I also added several more features: hooking up a fifth and sixth button
will allow you to clear the screen and toggle erase mode, respectively.
I also tidied up the display some.
