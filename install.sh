#!/bin/bash
echo Installing...
# make bin/ directory for executables
mkdir bin
# compile source files & create executables, link math library
gcc src/watchdog.c -lm -o bin/watchdog
gcc src/commander.c -lm -o bin/commander
gcc src/inspector.c -lm -o bin/inspector
gcc src/motorx.c -lm -o bin/motorx
gcc src/motorz.c -lm -o bin/motorz
touch run.sh
chmod +x run.sh;
# main executable script: run.sh
echo "#!/bin/bash" > run.sh
echo "gnome-terminal -- sh -c \"./bin/watchdog & ./bin/motorx & ./bin/motorz & ./bin/commander; bash\"" >> run.sh
echo "gnome-terminal -- sh -c \"./bin/inspector; bash\"" >> run.sh
echo "echo return value: \$?" >> run.sh

echo Installation complete. Executable created: run.sh
