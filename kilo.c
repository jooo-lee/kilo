// Tutorial from https://viewsourcecode.org/snaptoken/kilo/

/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

/* Bitwise-AND character with the value 00011111 (0x1f).
   This sets the upper 3 bits of the character to 0, which mirrors
   what the Ctrl key does in the terminal. */
#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

// Global struct that stores editor state
struct editorConfig E;

/*** terminal ***/

void die(const char *s) {
	// Clear screen and reposition cursor on exit
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	// Print error message and exit program with status of 1
	perror(s);
	exit(1);
}

// Restore user's original terminal attributes
void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) die("tcsetattr");
}

void enableRawMode() {
	// Store original terminal attributes before we make changes
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");

	// Call disableRawMode() automatically when program exits
	atexit(disableRawMode);

	// Read terminal attributes into termios struct
	struct termios raw = E.orig_termios;

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

// The easy way to get window size
int getWindowSize(int *rows, int *cols) {
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		return -1;
	} else {
		/* On success, the winsize struct will be populated with the
		 * correct number of rows and columns. We can then pass those
		 * values back by setting the int references that were passed
		 * to the function. */
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

/*** output ***/

void editorDrawRows() {
	// Print blank line for first line
	write(STDOUT_FILENO, "\n", 1);

	// Draw tildes at beginning of lines, like in Vim
	int y;
	for (y = 0; y < E.screenrows - 1; y++) {
		write(STDOUT_FILENO, "~", 1);

		// Don't print \r\n for last line
		if (y < E.screenrows - 2) {
			write(STDOUT_FILENO, "\r\n", 2);
		}
	}
}

void editorRefreshScreen() {
	// Clear user's screen using escape sequence
	write(STDOUT_FILENO, "\x1b[2J", 4);
	
	// Move cursor to first row for drawing rows
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	// Reposition cursor at top-left corner
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

// Waits for a keypress and handles it
void editorProcessKeypress() {
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			// Clear screen and reposition cursor on exit
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);

			exit(0);
			break;
	}
}

/*** init ***/ 

// Initialize all fields in the E struct
void initEditor() {
	if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main() {
	enableRawMode();
	initEditor();

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
