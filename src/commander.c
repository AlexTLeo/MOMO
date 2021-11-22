#include "../include/command.h"

/*
  The commander receives inputs from keys and sends velocity information to the
  hoist motors.
*/

// print commands and other useful info
void printInfo();

int main (int argc, char** argv) {
  float motorSpeedStep = 1;
  pid_t pid_watchdog;
  int fdx;
  int fdz;
  
  printf("Commander: booting up...\n");
  printf("Commander: running\n");
  fflush(stdout);

  // retrieving watchdog PID
  pid_watchdog = readPID("tmp/PID_watchdog");

  // opening pipes for motors x and z
  fdx = openPipeMotorComm("x");
  fdz = openPipeMotorComm("z");

  while (1) {
    // detecting user keypresses to control hoist
    clearTerminal();
    printIntro("Commander");
    printf("\n");
    printInfo();
    printf("\n\n");
    int input = detectKeyPress();

    switch (input) {
      case 97:
        // a: go left
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdx, -motorSpeedStep);
        break;
      case 100:
        // d: go right
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdx, motorSpeedStep);
        break;
      case 115:
        // s: go down
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdz, motorSpeedStep);
        break;
      case 119:
        // w: go up
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdz, -motorSpeedStep);
        break;
      case 120:
        // x: stop motorx (non-emergency)
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdx, 501);
        break;
      case 122:
        // z: stop motorz (non-emergency)
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdz, 501);
        break;
      default:
        // ignore all other keys
        break;
    }
  }

  return -1;
}

void printInfo() {
  terminalColor(31, true);
  printf("LEFT: ");
  terminalColor(37, true);
  printf("A");
  printf(" | ");
  terminalColor(31, true);
  printf("RIGHT: ");
  terminalColor(37, true);
  printf("D");
  printf(" | ");
  terminalColor(31, true);
  printf("UP: ");
  terminalColor(37, true);
  printf("W");
  printf(" | ");
  terminalColor(31, true);
  printf("DOWN: ");
  terminalColor(37, true);
  printf("S");
  printf(" | ");
  terminalColor(31, true);
  printf("HALT X AXIS: ");
  terminalColor(37, true);
  printf("X");
  printf(" | ");
  terminalColor(31, true);
  printf("HALT Z AXIS: ");
  terminalColor(37, true);
  printf("Z");

  fflush(stdout);
}
