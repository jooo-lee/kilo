// Tutorial from https://viewsourcecode.org/snaptoken/kilo/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

void enableRawMode() {
	// Read terminal attributes into termios struct
	struct termios raw;
	tcgetattr(STDIN_FILENO, &raw);

	/* 
	c_lflag is for "local flags" or "miscellaneous flags",
	one of which is the bitflag ECHO. Here we are flipping bits
	to turn off the ECHO feature.
	*/ 
	raw.c_lflag &= ~(ECHO);

	/* 
	Apply new terminal attributes to terminal.
	TCSAFLUSH specifies when to apply the change.
	In this case, it waits for all pending output to be written 
	to the terminal, and discards any input that hasn't been read.
	*/
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	/* 
	The terminal might still not echo even after the program exits.
	If that is the case, try typing ^c and typing 'reset' and pressing enter.
	We will fix this on the next step.
	*/ 
}

int main() {
	enableRawMode();
	
	char c;

	// Keep reading until end of file or 'q' is read
	while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
	return 0;
}
