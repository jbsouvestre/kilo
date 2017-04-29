// === INCLUDES === //
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// === DEFINES === //
#define CTRL_KEY(k) ((k) & 0x1f)

// === DATA === //
struct editor_config {
  struct termios orig_termios;
};

struct editor_config E;

// === TERMINAL === //

// error handling;
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disable_row_mode() {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enable_raw_mode() {
  if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
    die("tcsetattr");
  }
  // called when program exits
  atexit(disable_row_mode);

  struct termios raw = E.orig_termios;

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

char editor_read_key() {
  int nread;
  char c;
  while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if(nread == 1 && errno != EAGAIN) die("read");
  }
  return c;
}

int get_window_size(int *rows, int *cols) {
  struct winsize ws;

  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_cols;
    *rows = ws.ws_row;
    return 0;
  }
}

// === OUTPUT === //
void editor_draw_rows() {
  int y;
  for(y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editor_refresh_screen() {
  // clear screen
  //  write 4 bytes out to the TERMINAL
  // \x1b escape character
  // J clear the screen - 2 clear entire screen
  write(STDOUT_FILENO, "\x1b[2J", 4);

  // reposition cursor
  write(STDOUT_FILENO, "\x1b[H", 3);

  editor_draw_rows();
  write(STDOUT_FILENO, "\x1b[H", 3);

}



// === INPUT === //

void editor_process_keypress() {
  char c = editor_read_key();
  switch(c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

// === INIT === //

int main() {
  enable_raw_mode();

  while (1) {
    editor_refresh_screen();
    editor_process_keypress();
  }
  return 0;
}
