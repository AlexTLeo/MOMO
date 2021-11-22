#include "../include/motor.h"

/*
  Motor that moves the hoist along the Z axis. Sends estimated position back to
  inspector.
*/

int main (int argc, char** argv) {
  pid_t pid_watchdog;
  // retrieving watchdog PID
  pid_watchdog = readPID("tmp/PID_watchdog");

  motorLoop("z");
}
