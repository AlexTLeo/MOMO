#include "../include/command.h"

/*
  The commander receives inputs from keys and sends velocity information to the
  hoist motors.
*/

// print commands and other useful info
void printInfo();
void signalHandler (int signum);

pid_t pid_watchdog;
pid_t pid_inspector;
pid_t pid_inspector_sub;
int fdx;
int fdz;
int fdlog_info;
int fdlog_err;

int main (int argc, char** argv) {
  float motorSpeedStep = 1;

  // to detect shutdown request
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &signalHandler;
  sigaction(SIGUSR1, &sa, NULL);

  fdlog_info = openInfoLog();
  fdlog_err = openErrorLog();

  writeInfoLog(fdlog_info, "Commander: booting up...");
  writeInfoLog(fdlog_info, "Commander: running");

  // retrieving some PIDs
  pid_watchdog = readPID("tmp/PID_watchdog");
  pid_inspector_sub = readPID("tmp/PID_inspector_sub");
  pid_inspector = readPID("tmp/PID_inspector");

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
        writeInfoLog(fdlog_info, "Commander: move left command sent");
        break;
      case 100:
        // d: go right
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdx, motorSpeedStep);
        writeInfoLog(fdlog_info, "Commander: move right command sent");
        break;
      case 115:
        // s: go down
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdz, motorSpeedStep);
        writeInfoLog(fdlog_info, "Commander: move down command sent");
        break;
      case 119:
        // w: go up
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdz, -motorSpeedStep);
        writeInfoLog(fdlog_info, "Commander: move up command sent");
        break;
      case 120:
        // x: stop motorx (non-emergency)
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdx, 501);
        writeInfoLog(fdlog_info, "Commander: stop motorx command sent");
        break;
      case 122:
        // z: stop motorz (non-emergency)
        kill(pid_watchdog, SIGUSR1);
        commandMotor(fdz, 501);
        writeInfoLog(fdlog_info, "Commander: stop motorz command sent");
        break;
      case 113: ;
        // q: shut down simulation
        terminalColor(41, 1);
        printf("Commander: simulation SHUTDOWN in progress...");
        fflush(stdout);
        commandMotor(fdx, 502);
        commandMotor(fdz, 502);
        kill(pid_watchdog, SIGTERM);
        // inspector forked into two processes, kill both
        kill(pid_inspector, SIGTERM);
        kill(pid_inspector_sub, SIGTERM);
        writeInfoLog(fdlog_info, "Commander: shut down command sent");
        closePipe(fdx);
        closePipe(fdz);
        closeLog(fdlog_info);
        closeLog(fdlog_err);
        sleep(3);
        exit(0);
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
  printf("a");
  printf(" | ");
  terminalColor(31, true);
  printf("RIGHT: ");
  terminalColor(37, true);
  printf("d");
  printf(" | ");
  terminalColor(31, true);
  printf("UP: ");
  terminalColor(37, true);
  printf("w");
  printf(" | ");
  terminalColor(31, true);
  printf("DOWN: ");
  terminalColor(37, true);
  printf("s");
  printf(" | ");
  terminalColor(31, true);
  printf("HALT X AXIS: ");
  terminalColor(37, true);
  printf("x");
  printf(" | ");
  terminalColor(31, true);
  printf("HALT Z AXIS: ");
  terminalColor(37, true);
  printf("z");
  printf(" | ");
  terminalColor(31, true);
  printf("\n\nTERMINATE SIMULATION: ");
  terminalColor(37, true);
  printf("q");

  fflush(stdout);
}

void signalHandler (int signum) {
  if (signum == SIGUSR1) {
    // SHUT DOWN
    terminalColor(41, 1);
    printf("Commander: simulation SHUTDOWN in progress...\n");
    fflush(stdout);
    commandMotor(fdx, 502);
    commandMotor(fdz, 502);
    kill(pid_watchdog, SIGTERM);
    // inspector forked into two processes, kill both
    kill(pid_inspector, SIGTERM);
    kill(pid_inspector_sub, SIGTERM);
    writeInfoLog(fdlog_info, "Commander: shut down command sent");
    closePipe(fdx);
    closePipe(fdz);
    closeLog(fdlog_info);
    closeLog(fdlog_err);
    sleep(3);
    exit(0);
  }
}
