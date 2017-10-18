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
#include <semaphore.h>
#include <fcntl.h>

#include "shm.h"

const unsigned long int NANOSECOND = 1000000000; 
pid_t childpid;
int shmid,Mshmid;
shmClock *shinfo; // my system time
shmMsg *shmsginfo; // with termination time
shmClock init_time; // starting time for the process
key_t clock_key,msg_key;
//sem
sem_t *semlockp;
//With reference to text 14.8 program
#define FLAGS (O_CREAT | O_EXCL)
int getnamed(char *name, sem_t **sem, int val) {
while (((*sem = sem_open(name, FLAGS, 0644, val)) == SEM_FAILED) &&
(errno == EINTR)) ;
if (*sem != SEM_FAILED)
return 0;
if (errno != EEXIST)
return -1;
while (((*sem = sem_open(name, 0)) == SEM_FAILED) && (errno == EINTR)) ;
if (*sem != SEM_FAILED)
return 0;
return -1;
}
pid_t r_wait(int *stat_loc) {
pid_t retval;
while (((retval = wait(stat_loc)) == -1) && (errno == EINTR)) ;
return retval;
}

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
	msg_key = 666;
	srand(time(NULL));
	//Read shared memory segment 
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

//Read shared memory segment for msgs
	Mshmid = shmget(msg_key, 20*sizeof(shmsginfo), 0744|IPC_EXCL);
	if ((Mshmid == -1) && (errno != EEXIST)) 
	{
		perror("Unable to read shared memory");
		return -1;
	} 
	else
	{
		shmsginfo = (shmMsg*)shmat(Mshmid,NULL,0);
		if (shmsginfo == (void*)-1)
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

  	if (getnamed("sandhya_vaithy", &semlockp, 1) == -1) {
perror("Failed to create named semaphore");
return 1;
}
while (sem_wait(semlockp) == -1) /* entry section */
if (errno != EINTR) {
perror("Failed to lock semlock");
return 1;
}

  	//critical section
  	fprintf(stderr, "critical section entered..");
  	fprintf(stderr, "Start time: %lu nsec: %lu \n", init_time.sec, init_time.nsec);


 	  while(1){
  	  	unsigned long long nsecdiff = (shinfo->sec - init_time.sec) * NANOSECOND + (shinfo->nsec - init_time.nsec);
        if (nsecdiff >= duration && (shmsginfo->term_ns == 0 && shmsginfo ->term_s == 0)) {
        	shmsginfo->term_ns = shinfo->nsec;
        	shmsginfo->term_s = shinfo->sec;
        	shmsginfo->process_id = getpid();
        	fprintf(stderr, "%d inside critical section \n",shmsginfo->process_id );
        }
        else
        	break;
  	  }

if (sem_post(semlockp) == -1) { /* exit section */
perror("Failed to unlock semlock");
return 1;
}
if (r_wait(NULL) == -1) /* remainder section */
return 1;

	return 0;
}


