#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disable_row_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  // called when program exits
  atexit(disable_row_mode);

  struct termios raw = orig_termios;

  // input flags
  raw.c_iflag &= ~(IXON);

  // ECHO: all keys are printed into the terminal
  // ICANON: turn off canonical mode (read inpyt byte byte byte instead of line by line)
  // c_lflag: dumping group for other states
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enable_raw_mode();
  char c;

  while (read(STDIN_FILENO, &c, 10) > 0 && c != 'q') {
    if (isprint(c)) {
      printf("%d ('%c')\n", c, c);
    } else {
      printf("%d\n", c);
    }
  }
  return 0;
}
