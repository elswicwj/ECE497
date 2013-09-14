TOOLS_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes
#
# Programs
#
all:	etch-external etch-keyboard control_leds

etch-external:  etch-external.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^

etch-keyboard:	etch-keyboard.o
	$(CC) $(LDFLAGS) -o $@ $^

control_leds:	control_leds.o gpio-utils.o
	$(CC) $(LDFLAGS) -o $@ $^


#
# Objects
#

%.o: %.c
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

clean:
	rm etch-external.o etch-keyboard.o control_leds.o gpio-utils.o
