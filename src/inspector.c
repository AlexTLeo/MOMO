#include <signal.h>

#include "../include/command.h"

/*
  The inspector outputs an estimate of the hoist/joist position in real time.
  It can also receive a RESET command that will reset the hoist, or an
  emergency STOP command that will halt the hoist in its place.
  The process forks into two:
  - the parent process always waits for emergency commands RESET and STOP
  - the child process keeps displaying coordinate information to terminal
*/

void signalHandler (int signum);
// graphical representation of the hoist
void drawHoist(float coordx, float coordz, float downsizeFactor);
// prints useful information (commands, current velocity, etc.)
void printInfo(float coordx, float coordz);

int fdmc_x;
int fdmc_z;

int main (int argc, char** argv) {
  pid_t pid_watchdog;
  pid_t pid_child;
  pid_t pid_motorx;
  pid_t pid_motorz;
  float simulationSpeed = SIM_SPEED;

  // printf("Inspector: booting up...\n");
  // printf("Inspector: running\n");
  // fflush(stdout);

  // retrieving watchdog PID
  pid_watchdog = readPID("tmp/PID_watchdog");

  // The first line is the PID of the motor
  pid_motorx = readPID("tmp/PID_motorx");
  pid_motorz = readPID("tmp/PID_motorz");
  // printf("Inspector: motor processes detected (PID_X: %d, PID_Z: %d)\n", pid_motorx, pid_motorz);
  // fflush(stdout);

  pid_child = fork();
  if (pid_child != 0) {
    // PARENT
    struct sigaction sa;

    // sending inspector subprocess PID to watchdog
    writePID("tmp/PID_inspector", true);

    // printf("Inspector: awaiting commands...\n");
    // fflush(stdout);
    // Opening pipes for motors x and z
    fdmc_x = openPipeMotorComm("x");
    fdmc_z = openPipeMotorComm("z");

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signalHandler;
    sigaction(SIGUSR1, &sa, NULL);

    while (1) {
      // Detecting user keypresses for RESET and EMERGENCY STOP buttons
      int input = detectKeyPress();

      switch (input) {
        case 32:
          // spacebar: EMERGENCY STOP
          // signal the watchdog to send its PID to the new motors
          kill(pid_watchdog, SIGUSR2);

          // printf("Inspector: EMERGENCY STOP...\n");
          // fflush(stdout);

          // immediately kill motorx and motorz (using SIGKILL for safety reasons:
          // the hoist must stop IMMEDIATELY) and then restart the two processes
          kill(pid_motorx, SIGKILL);
          kill(pid_motorz, SIGKILL);

          // printf("Inspector: Motors have been stopped.\n");
          // printf("Inspector: Motors reinitialization in progress...\n");
          // fflush(stdout);

          // exec motorx motorz
          char* arg_listx[] = {"./bin/motorx", NULL};
          char* arg_listz[] = {"./bin/motorz", NULL};

          if (fork() == 0) {
            execvp("./bin/motorx", arg_listx);
          }

          if (fork() == 0) {
            execvp("./bin/motorz", arg_listz);
          }

          // read PID
          pid_motorx = readPID("tmp/PID_motorx");
          pid_motorz = readPID("tmp/PID_motorz");
          // printf("Inspector: motor processes detected (PID_X: %d, PID_Z: %d)\n", pid_motorx, pid_motorz);
          // fflush(stdout);
          //
          // printf("Inspector: motors have been re-initialized\n");
          // printf("Inspector: awaiting commands...\n");
          // fflush(stdout);

          break;
        case 114:
          // r: RESET
          kill(pid_watchdog, SIGUSR1);
          // printf("Inspector: RESET signal sent\n");
          // fflush(stdout);

          commandMotor(fdmc_x, 500);
          commandMotor(fdmc_z, 500);
          break;
        default:
          // Ignore all other keys
          break;
      }
    }
  } else {
    // CHILD
    int fdmi_x;
    int fdmi_z;
    float coordx;
    float coordz;

    fdmi_x = openPipeMotorInspector("x");
    fdmi_z = openPipeMotorInspector("z");

    while (1) {
      clearTerminal();
      coordx = readCoordinates(fdmi_x);
      coordz = readCoordinates(fdmi_z);
      printIntro("Inspector");
      printf("\n");
      drawHoist(coordx, coordz, 1.5f);
      printf("\n\n");
      printInfo(coordx, coordz);

      usleep(simulationSpeed);
    }
  }
}

void signalHandler (int signum) {
  if (signum == SIGUSR1) {
    // RESET
    commandMotor(fdmc_x, 500);
    commandMotor(fdmc_z, 500);
  }
}

void drawHoist(float coordx, float coordz, float downsizeFactor) {
  float downsizeFactorX = downsizeFactor;
  float downsizeFactorZ = downsizeFactor*5;
  float posX = round(coordx/downsizeFactorX);
  float posZ = round(coordz/(downsizeFactorZ)); // need more vertical downsize
  int numSpacesHook = 0;
  // drawing the X axis and the hoist body
  for (int i = 0; i < MAX_X/downsizeFactorX; i++) {
    if (posX == i) {
      // body's current position
      terminalColor(33, true);
      printf("H");
    } else {
      // track
      terminalColor(37, true);
      if (i % 2 == 0) {
        printf("=");
      } else {
        printf("-");
      }
    }
  }

  // drawing the Z axis and the hoist hook
  for (int i = 0; i < posZ; i++) {
    // printing spaces to align hook to hoist body
    printf("\n%*s", (int) posX, "");
    // track, or "cable"
    printf("|");
  }

  // hook is the last thing to print
  terminalColor(33, true);
  printf("\n%*s", (int) posX, "");
  printf("J");

  // show lowest point hook can go
  terminalColor(30, 1);
  for (int i = posZ; i < MAX_Z/(downsizeFactorZ); i++) {
    printf("\n");
  }
  for (int i = 0; i < MAX_X/downsizeFactorX; i++) {
    printf("_");
  }

  fflush(stdout);
}

void printInfo(float coordx, float coordz) {
  terminalColor(31, true);
  printf("RESET HOIST: ");
  terminalColor(37, true);
  printf("R");
  printf(" | ");
  terminalColor(41, true);
  printf("EMERGENCY STOP: ");
  terminalColor(37, true);
  printf("spacebar");

  terminalColor(0, false);
  terminalColor(34, true);
  printf("       ");
  printf("X: %.1f", coordx);
  printf(" | ");
  printf("Z: %.1f", coordz);

  fflush(stdout);
}
