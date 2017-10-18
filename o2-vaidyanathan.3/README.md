# oss - PROJECT 3
SANDHYA VAIDYANATHAN
GITHUB: https://github.com/SandhyaVaidyanathan/oss

HELP:
 ./oss -h
Usage executable -s {no. of slave processes to be spawned} -l {log file name} -t {time in seconds when master will terminate with all children}

Executable example:
./oss -s 5 -t 60 -l "log.txt"

shm.h - contains struct for osclock and shmmsg
oss.c - forks off child processes and calls user.c
user.c - handles semaphore 

