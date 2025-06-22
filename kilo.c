// Tutorial from https://viewsourcecode.org/snaptoken/kilo/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

// Print error message and exit program with status of 1
void die(const char *s) {
	perror(s);
	exit(1);
}

void disableRawMode() {
	// Restore user's original terminal attributes
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
	// Store original terminal attributes before we make changes
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");

	// Call disableRawMode() automatically when program exits
	atexit(disableRawMode);

	// Read terminal attributes into termios struct
	struct termios raw = orig_termios;

	// Disable flags to enable raw mode
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	raw.c_oflag &= ~(OPOST);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	raw.c_cflag |= (CS8);
	
	// Set 1/10 second timeout for read()
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	// Apply new terminal attributes to terminal
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int main() {
	enableRawMode();

	while (1) {
		/* If we don't supply any input, the program will
		   simply keep printing out 0s. */
		char c = '\0';

		/* In Cygwin, when read() times out it returns -1 with errno of EAGAIN
		   instead of returning 0. So we don't treate EAGAIN as an error to make
		   this work in Cygwin. */
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
		/* Test whether a character is a control character. 
		   Control characters are nonprintable characters that we don't
		   want to print to the screen, e.g. ESC or TAB. */
		if (iscntrl(c)) {
			/* %d tells printf() to format the byte as a decimal 
			   number (its ASCII code). */
			printf("%d\r\n", c);
		} else {
			/* %c tells printf() to write out the byte directly,
			   as a character. */
			printf("%d ('%c')\r\n", c, c);
		}
		
		// Exit program
		if (c == 'q') break;
	}

	return 0;
}
