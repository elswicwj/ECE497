# Created 9/19/2013 by Will Elswick using gedit for the Beagle Bone Black

if [ "$#" -ne 2 ]; then
	echo "Usage: $0 <I2C_bus> <register_address>"
	exit 1
fi

TEMP=$(i2cget -y $1 $2 0)
declare -i TEMP

if [ -z "$TEMP" ]; then
	echo "Temperature sensor not found."
	exit 1
fi

#temp now in hex, celsius

TENS=$TEMP/10
ONES=$TEMP%10
TEMP=$TENS*10+$ONES

#temp now in decimal, celsius

TEMP=18\*$TEMP/10+32

#temp now in fahrenheit

echo The temperature in fahrenheit is: $TEMP
