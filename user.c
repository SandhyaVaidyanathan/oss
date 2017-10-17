#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <getopt.h>

#include "shm.h"

pid_t childpid;
int shmid;
shmClock *shinfo; // my system time
shmClock init_time; // starting time for the process
key_t clock_key;

void interruptHandler(int SIG){
  signal(SIGQUIT, SIG_IGN);
  signal(SIGINT, SIG_IGN);

  if(SIG == SIGINT)
   {
    fprintf(stderr, "\nCTRL-C encoutered, killing processes\n");
  	}

  if(SIG == SIGALRM) 
  {
    fprintf(stderr, "Master has timed out. killing processes\n");
  }

	kill(-getpgrp(), SIGQUIT);
}

int main(int argc, char const *argv[])
{
	int processnumber = atoi(argv[0]);
	clock_key = 555;
	srand(time(NULL));
	//Create shared memory segment 
	shmid = shmget(clock_key, 20*sizeof(shinfo), 0744|IPC_EXCL);
	if ((shmid == -1) && (errno != EEXIST)) 
	{
		perror("Unable to read shared memory");
		return -1;
	} 
	else
	{
		shinfo = (shmClock*)shmat(shmid,NULL,0);
		if (shinfo == (void*)-1)
		{
			printf("Cannot attach shared memory\n");
			return -1;
		}
	}

	// reading the simulated time into init_time
	init_time.sec = shinfo->sec;
    init_time.nsec = shinfo->nsec;
    //generating random duration
  	long long duration = 1 + rand() % 100000;

  	//critical section
  	fprintf(stderr, "critical section entered..");


	return 0;
}