#include <stdbool.h>
#include <math.h>

/*
  Common functions and variables across all processes
*/

// motor sleeps for SIM_SPEED microseconds on each position increment
#define SIM_SPEED 2000000
// hoist range: [0;MAX_X] and [0;MAX_Z]
#define MAX_X 100
#define MAX_Z 100

// Writes PID to a pipe
void writePID(char* pipeName, bool isClosing) {
  pid_t pid =  getpid();
  int fd;

  if (mkfifo(pipeName, 0666) == -1 && errno != 17) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writePID mkfifo");
  }

  fd = open(pipeName, O_WRONLY);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writePID pipe open");
  }

  if (write(fd, &pid, sizeof(pid)) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writePID write");
  }

  if (isClosing) {
    if (close(fd) == -1) {
      printf("Error %d in ", errno);
      fflush(stdout);
      perror("common.h writePID close");
    }
  }
}

pid_t readPID (char* pipeName) {
  int fd;
  pid_t pid;

  // (ignore "file already exists", errno 17)
  if (mkfifo(pipeName, 0666) == -1 && errno != 17) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h readPID mkfifo");
  }

  fd = open(pipeName, O_RDONLY);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h readPID open");
  }

  if(read(fd, &pid, sizeof(pid)) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h readPID read");
  }

  if (close(fd) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("command.h readPID close");
  }

  return pid;
}

// clears the terminal
void clearTerminal() {
  printf("\033c");
  system("clear");
  fflush(stdout);
}

// changes terminal color
// colorCode - ANSI color code
void terminalColor(int colorCode, bool isBold) {
  char* specialCode = "";
  if (isBold) {
    specialCode = "1;";
  }

  printf("\033[%s%dm", specialCode, colorCode);
  fflush(stdout);
}

void printIntro(char* consoleName) {
  terminalColor(32, true);
  printf("MOMO: [M]ark 1 [O]rbital [M]anipulator of [O]bjects\n");
  terminalColor(32, false);
  printf("Space Hoist Satellite\n");
  terminalColor(35, true);
  printf("\n%s Console\n", consoleName);
  fflush(stdout);
}
