// === INCLUDES === //
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

// === TERMINAL === //

// error handling;
void die(const char *s) {
  perror(s);
  exit(1);
}

void disable_row_mode() {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enable_raw_mode() {
  if(tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
    die("tcsetattr");
  }
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

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

// === INIT === //

int main() {
  enable_raw_mode();

  while (1) {
    char c = '\0';
    if( read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN ){
      die("read");
    }
    if (isprint(c)) {
      printf("%d ('%c')\r\n", c, c);
    } else {
      printf("%d\r\n", c);
    }
    if(c == 'q') break;
  }
  return 0;
}
