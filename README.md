**Institution:** Università di Genova<br>
**Course:** MSc in Robotics Engineering<br>
**Subject:** Research Track 1<br>
**Author:** ***Alex Thanaphon Leonardi***<br>

# MOMO: [M]ark 1 [O]rbital [M]anipulator of [O]bjects
This program, written in C, comprises 5 different processes that work together and communicate together via IPC (signals and pipes) to simulate the mechanisms behind an industrial hoist.
(For my own entertainment, I imagined this hoist to be attached to a satellite).

## Running The Program
To run the program, gcc is required to compile the source code.
```
sudo apt-get install gcc
```

Then, navigate to the ***momo*** directory and run the install script.
```
cd example/momo/
./install.sh
```

Then, simply execute the **run** file!
```
./run
```

## Behind The Scenes...
The program consists of 5 processes that work together:
1. watchdog
2. commander
3. inspector
4. motorx
5. motorz

The program runs through *simulation cycles*, whose speed is determined in the **common.h** header file. For example, a **SIM_SPEED** of 2000000 microseconds means that the program recalculates its state every 0.2 seconds.

### 1. Watchdog
The watchdog process monitors all other processes by waiting for an OK signal from any one of them. If no OK signal arrives by **RESET_TIME** (as defined in watchdog.c), then a RESET signal is sent to the **inspector process**, who proceeds to reset the hoist back to its original position.

### 2. Commander
The commander process awaits for user input and sends commands to the **motorx** and **motorz** processes. The commands are sent via pipes.

### 3. Inspector
The inspector process displays relevant information to the user (a graphical representation of the hoist, along with its numerical coordinates) and also waits for two special commands: **RESET**, which brings the hoist back to its starting position, and **EMERGENCY STOP** which kills the **motorx** and **motorz** processes and relaunches them. Specifically, **RESET** sends a command via pipe to the motors, while **EMERGENCY STOP** sends a SIGKILL signal to the motors and relaunches them via a fork-exec mechanism.  

### 4&5. MotorX and MotorZ
These two processes simply receive velocity commands and calculate a new position every simulation cycle, plus a randomized error that is added onto the actual position and serves the purpose of simulating a real-life measurement error due to sensors' physical limitations and other disturbances.
A velocity command of **500** corresponds to the **RESET** command, **501** corresponds to the **non-emergency stop** command.

## Conclusion
This was a very interesting assignment as it allowed for a more practical view of how C and its IPC mechanisms could be used in a real life scenario.
