// Tutorial from https://viewsourcecode.org/snaptoken/kilo/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode() {
	// Restore user's original terminal attributes
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
	// Store original terminal attributes before we make changes
	tcgetattr(STDIN_FILENO, &orig_termios);

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
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
	enableRawMode();

	while (1) {
		/* If we don't supply any input, the program will
		   simply keep printing out 0s. */
		char c = '\0';

		read(STDIN_FILENO, &c, 1);
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
