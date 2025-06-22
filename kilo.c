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

	// Modify flags to enable raw mode
	raw.c_iflag &= ~(IXON | ICRNL);
	raw.c_oflag &= ~(OPOST);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	// Apply new terminal attributes to terminal
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
	enableRawMode();

	char c;

	// Keep reading until end of file or 'q' is pressed
	while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
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
	}

	return 0;
}
