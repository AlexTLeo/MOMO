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
#include <math.h>

#include "../include/common.h"

/*
  Header file for all motors
*/

// Closes the specified pipe
void closePipe(int fd) {
  if (close(fd) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("motor.h closePipe");
  }
}

// Creates and opens the pipes
// fd[0] commanderPipeName, fd[1] inspectorPipeName
void activateMotor(int fd[2], char* commanderPipeName, char* inspectorPipeName) {

  // COMMANDER pipe
  // (ignore "file already exists", errno 17)
  if (mkfifo(commanderPipeName, 0666) == -1 && errno != 17) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("motor.h commander mkfifo");
  }

  fd[0] = open(commanderPipeName, O_RDONLY);
  if (fd[0] == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("motor.h commander pipe open");
  }

  // INSPECTOR pipe
  // (ignore "file already exists", errno 17)
  if (mkfifo(inspectorPipeName, 0666) == -1 && errno != 17) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("motor.h inspector mkfifo");
  }

  fd[1] = open(inspectorPipeName, O_WRONLY);
  if (fd[1] == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("motor.h inspector pipe open");
  }
}

// Reads speed from pipe sent by commander process
// If 0 is returned, then that means there was no data to read. Ignore it.
float readSpeed(int fd) {
  float speed = 0;

  // for the select()
  fd_set fdSet;
  int retval;
  int maxfd;
  struct timeval tv;
  tv.tv_sec = 0;  // we don't want any blocking!
  tv.tv_usec = 0;

  FD_ZERO(&fdSet);
  FD_SET(fd, &fdSet);
  maxfd = fd;

  retval = select(maxfd + 1, &fdSet, NULL, NULL, &tv);
  if (retval == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("motor.h select");
  } else if (retval) {
    if (FD_ISSET(fd, &fdSet)) {
      // read speed from pipe
      read(fd, &speed, sizeof(speed));
      // if 0 was read, that means it is EOF
      if (speed == 0) {
        // EOF
      }
    }
  } else {
    // nothing to read
  }

  return speed;
}

void writeCoordinates(int fd, float coordinates) {
  if (write(fd, &coordinates, sizeof(coordinates)) == -1) {
    printf("Error %d in ", errno);
    fflush(stdout);
    perror("motor.h coordinate write");
  }
}

// Main loop that updates position and reads new commands from commander
void motorLoop (char* axis) {
  int fd[2]; // fd[0] commanderPipe, fd[1] inspectorPipe
  int maxAxis;
  bool isStopped = false;
  char *commanderPipeName;
  char *inspectorPipeName;
  char *pidPipeName;
  float position = 0; // start from leftmost position on track
  float simulationSpeed = SIM_SPEED;
  float estimatedPosition;
  float newSpeedStep = 0;
  float currentSpeed = 0;

  srand(time(0));

  // selecting correct motor properties
  if (axis == "x") {
    maxAxis = MAX_X;
    commanderPipeName = "tmp/motorcommands_x";
    inspectorPipeName = "tmp/motorinspector_x";
    pidPipeName = "tmp/PID_motorx";
  } else if (axis == "z") {
    maxAxis = MAX_Z;
    commanderPipeName = "tmp/motorcommands_z";
    inspectorPipeName = "tmp/motorinspector_z";
    pidPipeName = "tmp/PID_motorz";
  } else {
    printf("Error %d in ", errno);
    perror("motorLoop axis selection");
  }

  // Send PID to inspector
  writePID(pidPipeName, true);

  activateMotor(fd, commanderPipeName, inspectorPipeName);

  while (1) {
    newSpeedStep = readSpeed(fd[0]);
    if (newSpeedStep != 0) {
      // printf("Motor%s new speed read: %f\n", axis, newSpeedStep);
      // fflush(stdout);
    }

    if (round(newSpeedStep) == 500) {
      // RESET command
      // printf("Motor%s: RESET received\n", axis);
      // fflush(stdout);
      currentSpeed = 0;
      position = -maxAxis; // FIXME: too sudden of a change
    } else if (isStopped) {
      // EMERGENCY STOP command has been signalled
    } else if (round(newSpeedStep) == 501) {
      // NON-EMERGENCY STOP
      currentSpeed = 0;
    } else {
      // normal motor movement
      if (newSpeedStep == 0) {
        // pipe EOF - ignore this value
      } else {
        currentSpeed += newSpeedStep;
      }

      position += currentSpeed;
    }

    if (fabs(position) > maxAxis || position < 0) {
      // reached end of track
      // minor correction to make sure position is always within bounds
      if (position > 0) {
        position = maxAxis;
      } else {
        position = 0;
      }
    }

    // send current coordinate estimate to the inspector (with error)
    float error = ((float)rand()/(float)(RAND_MAX)) - 0.5f;
    estimatedPosition = position + error;

    if (estimatedPosition < 0) {
      estimatedPosition = 0.0f;
    } else if (estimatedPosition > 100) {
      estimatedPosition = 100.0f;
    }

    writeCoordinates(fd[1], estimatedPosition);

    // sleep for a preset amount of time, to simulate a real motion.
    usleep(simulationSpeed);
  }
}
