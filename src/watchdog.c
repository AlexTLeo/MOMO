#include "../include/command.h"

/*
  Monitors all processes (commander, inspector, motorx, motorz) and waits for a
  periodic OK signal from any of the processes. If no OK signal is received for
  the specified RESET_TIME (seconds), then the hoist is RESET (request the
  inspector to send a RESET command to the motors).
*/

void signalHandler (int signum);

#define RESET_TIME 60

pid_t pid_inspector;
bool isKiller = true;

int main (int argc, char** argv) {
  struct sigaction sa;
  char* pipeNameInspector = "tmp/PID_inspector";
  int fdlog_info;
  int fdlog_err;

  fdlog_info = openInfoLog();
  fdlog_err = openErrorLog();

  writeInfoLog(fdlog_info, "Watchdog: booting up...");

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &signalHandler;
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGUSR2, &sa, NULL);

  writeInfoLog(fdlog_info, "Watchdog: running");

  sleep(1); // required to avoid deadlock (not 100% sure why...)

  writeInfoLog(fdlog_info, "Watchdog: sending PID to other processes...");
  writePID("tmp/PID_watchdog", false); // for commander
  writePID("tmp/PID_watchdog", false); // for inspector
  writePID("tmp/PID_watchdog", false); // for motorx
  writePID("tmp/PID_watchdog", true); // for motorz
  writeInfoLog(fdlog_info, "Watchdog: PID has been sent to other processes");

  pid_inspector = readPID(pipeNameInspector);
  writeInfoLog(fdlog_info, "Watchdog: inspector PID received");

  while (1) {
    // printf("Watchdog: RESET signal timer started...\n");
    // fflush(stdout);
    isKiller = true;

    sleep(RESET_TIME);

    if (isKiller) {
      writeInfoLog(fdlog_info, "Watchdog: RESET signal sent");
      // printf("Watchdog: RESET signal sent\n");
      // fflush(stdout);
      kill(pid_inspector, SIGUSR1);
    }
  }
}

void signalHandler (int signum) {
  if (signum == SIGUSR1) {
    // printf("Watchdog: RESET signal interrupted\n");
    // fflush(stdout);
    isKiller = false;
  }

  if (signum == SIGUSR2) {
    // printf("Watchdog: RESET signal interrupted\n");
    // fflush(stdout);
    isKiller = false;
    writePID("tmp/PID_watchdog", false); // for motorx
    writePID("tmp/PID_watchdog", true); // for motorz
  }
}
