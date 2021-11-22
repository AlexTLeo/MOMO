#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>

#include "../include/common.h"

/*
  Header file for all command modules (commander, inspector)
*/

// detects key presses, without waiting for newline (ENTER)
int detectKeyPress () {
  int input;
  struct termios orig_term_attr;
  struct termios new_term_attr;

  /* set the terminal to raw mode rather than canonical mode */
  tcgetattr(fileno(stdin), &orig_term_attr);
  memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
  new_term_attr.c_lflag &= ~(ECHO|ICANON);
  new_term_attr.c_cc[VTIME] = 0;
  new_term_attr.c_cc[VMIN] = 1;
  tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

  /* read a character from the stdin stream without blocking */
  /* returns EOF (-1) if no character is available */
  input = getchar();

  /* restore the original terminal attributes */
  tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

  return input;
}

// Writes to pipe the speed and axis specified
void commandMotor (int fd, float speed) {
  if (write(fd, &speed, sizeof(speed)) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h motorcomm write");
  }
}

// Creates and opens the COMMANDER pipe
int openPipeMotorComm(char *axis) {
  int fd;
  char pipeName[32] = "tmp/motorcommands_";
  strcat(pipeName, axis); // e.g. motorcommands_x

  // (ignore "file already exists", errno 17)
  if (mkfifo(pipeName, 0666) == -1 && errno != 17) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h mkfifo");
  }

  fd = open(pipeName, O_WRONLY);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h motorcomm open");
  }

  return fd;
}

// Closes the pipe
void closePipeMotorComm(int fd) {
  if (close(fd) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h motorcomm close");
  }
}

// Creates and opens the INSPECTOR pipe
int openPipeMotorInspector(char *axis) {
  int fd;
  char pipeName[32] = "tmp/motorinspector_";
  strcat(pipeName, axis);

  // (ignore "file already exists", errno 17)
  if (mkfifo(pipeName, 0666) == -1 && errno != 17) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h motorinspector mkfifo");
  }

  fd = open(pipeName, O_RDONLY);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h motorinspector open");
  }

  return fd;
}

// Closes the pipe
void closePipeMotorInspector (int fd) {
  if (close(fd) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h motorinspector close");
  }
}

float readCoordinates (int fd) {
  float coordinate;

  if(read(fd, &coordinate, sizeof(coordinate)) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h motorinspector read");
  }

  return coordinate;
}
