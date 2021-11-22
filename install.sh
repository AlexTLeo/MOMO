#!/bin/bash
gcc src/watchdog.c -lm -o bin/watchdog
gcc src/commander.c -lm -o bin/commander
gcc src/inspector.c -lm -o bin/inspector
gcc src/motorx.c -lm -o bin/motorx
gcc src/motorz.c -lm -o bin/motorz
touch run
chmod +x run;
echo "gnome-terminal -- sh -c \"./bin/watchdog & ./bin/motorx & ./bin/motorz & ./bin/commander\"" > run
echo "gnome-terminal -- sh -c \"./bin/inspector\"" >> run
