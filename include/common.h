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
#include <math.h>
#include <sys/file.h>

/*
  Common functions and variables across all processes
*/
void writePID(char* pipeName, bool isClosing);
pid_t readPID (char* pipeName);
void clearTerminal();
void terminalColor(int colorCode, bool isBold);
void printIntro(char* consoleName);
int openInfoLog();
void writeInfoLog(int fd, char* string);
void writeErrorLog(int fd, char* string);
void closeLog(int fd);

// log file descriptors
int fdlog_info;
int fdlog_err;

// motor sleeps for SIM_SPEED microseconds on each position increment
#define SIM_SPEED 200000
// hoist range: [0;MAX_X] and [0;MAX_Z]
#define MAX_X 100
#define MAX_Z 100
// used to write to logs
#define BUFF_SIZE 8192

// Writes PID to a pipe
void writePID(char* pipeName, bool isClosing) {
  pid_t pid =  getpid();
  int fd;

  if (mkfifo(pipeName, 0666) == -1 && errno != 17) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writePID mkfifo");
    writeErrorLog(fdlog_err, "common.h: writePID mkfifo failed");
    exit(-1);
  }

  fd = open(pipeName, O_WRONLY);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writePID pipe open");
    writeErrorLog(fdlog_err, "common.h: writePID open failed");
    exit(-1);
  }

  if (write(fd, &pid, sizeof(pid)) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writePID write");
    writeErrorLog(fdlog_err, "common.h: writePID write failed");
    exit(-1);
  }

  if (isClosing) {
    if (close(fd) == -1) {
      printf("Error %d in ", errno);
      fflush(stdout);
      perror("common.h writePID close");
      writeErrorLog(fdlog_err, "common.h: writePID close failed");
      exit(-1);
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
    perror("common.h readPID mkfifo");
    writeErrorLog(fdlog_err, "common.h: readPID mkfifo failed");
    exit(-1);
  }

  fd = open(pipeName, O_RDONLY);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h readPID open");
    writeErrorLog(fdlog_err, "common.h: readPID open failed");
    exit(-1);
  }

  if(read(fd, &pid, sizeof(pid)) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h readPID read");
    writeErrorLog(fdlog_err, "common.h: readPID read failed");
    exit(-1);
  }

  if (close(fd) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h readPID close");
    writeErrorLog(fdlog_err, "common.h: readPID close failed");
    exit(-1);
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

// opens error log
int openErrorLog() {
  int fd;

  fd = open("logs/errors.log", O_WRONLY | O_APPEND | O_CREAT, 0666);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h openErrorLog");
    exit(-1);
  }

  return fd;
}

// opens info log
int openInfoLog() {
  int fd;

  fd = open("logs/info.log", O_WRONLY | O_APPEND | O_CREAT, 0666);
  if (fd == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h openInfoLog");
    exit(-1);
  }

  return fd;
}

// writes to info log
void writeInfoLog(int fd, char* string) {
  // get current time
  time_t rawtime;
  struct tm * timeinfo;
  char* currentTime = malloc(sizeof(timeinfo));
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  // prettier format...
  sprintf(currentTime, "[%d-%d-%d %d:%d:%d]", timeinfo->tm_mday,
      timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour,
      timeinfo->tm_min, timeinfo->tm_sec);

  // make sure print is atomic
  flock(fd, LOCK_EX);
  if (dprintf(fd, "%s %s\n", currentTime, string) < 0) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writeInfoLog");
    exit(-1);
  }
  flock(fd, LOCK_UN);
}

// writes to error log
void writeErrorLog(int fd, char* string) {
  // get current time
  time_t rawtime;
  struct tm * timeinfo;
  char* currentTime = malloc(sizeof(timeinfo));
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  // prettier format...
  sprintf(currentTime, "[%d-%d-%d %d:%d:%d]", timeinfo->tm_mday,
      timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour,
      timeinfo->tm_min, timeinfo->tm_sec);

  // make sure print is atomic
  flock(fd, LOCK_EX);
  if (dprintf(fd, "%s %s\n", currentTime, string) < 0) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h writeErrorLog");
    exit(-1);
  }
  flock(fd, LOCK_UN);
}

// closes log defined by fd
void closeLog(int fd) {
  if (close(fd) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h closeLog");
    exit(-1);
  }
}

// Closes the specified pipe
void closePipe(int fd) {
  if (close(fd) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("common.h closePipe");
    writeErrorLog(fdlog_err, "common.h: closePipe failed");
    exit(-1);
  }
}
