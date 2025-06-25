// Tutorial from https://viewsourcecode.org/snaptoken/kilo/

/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

/* Bitwise-AND character with the value 00011111 (0x1f).
   This sets the upper 3 bits of the character to 0, which mirrors
   what the Ctrl key does in the terminal. */
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

// Print error message and exit program with status of 1
void die(const char *s) {
	perror(s);
	exit(1);
}

// Restore user's original terminal attributes
void disableRawMode() {
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

// Wait for one keypress and return it 
char editorReadKey() {
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

/*** output ***/

void editorRefreshScreen() {
	// Clear user's screen using escape sequence
	write(STDOUT_FILENO, "\x1b[2J", 4);
	
	// Reposition cursor at top-left corner
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

// Waits for a keypress and handles it
void editorProcessKeypress() {
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			exit(0);
			break;
	}
}

/*** init ***/ 

int main() {
	enableRawMode();

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
