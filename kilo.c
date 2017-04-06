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
  // IXON: disable Ctrl+s Ctrl+q
  // ICRNL Fix Ctrl+m
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  raw.c_cflag |= (CS8);

  // fix output
  raw.c_oflag &= ~(OPOST);
  // ECHO: all keys are printed into the terminal
  // ICANON: turn off canonical mode (read inpyt byte byte byte instead of line by line)
  // ISIG:  turn off Ctrl+c Ctrl+
  // IEXTEN: disable Ctrl+v
  // c_lflag: dumping group for other states
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enable_raw_mode();

  while (1) {
    char c = '\0';
    read(STDIN_FILENO, &c, 1);
    if (isprint(c)) {
      printf("%d ('%c')\r\n", c, c);
    } else {
      printf("%d\r\n", c);
    }
    if(c == 'q') break;
  }
  return 0;
}
