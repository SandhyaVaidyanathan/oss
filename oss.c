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

int spawnedSlaves = 0;
pid_t childpid, gpid;
const int ZTIME_DEFAULT = 20;
const int MAXSLAVE_DEFAULT = 5;

void spawnSlaveProcess(int);
void interruptHandler(int);

int main(int argc, char const *argv[])
{
	char* logfile;
	logfile = "log.txt";
	int option = 0;	
	int slaveProcess = 0,ztime = 0 ;
	int shmid;
	shmClock *shinfo;
	key_t key;

	if (argc < 2){ // check for  command-line arguments		
  		fprintf(stderr, " %s: Error : Try Executable -h for help \n",argv[0]);		
  	return 1;	
  	}
  	while ((option = getopt(argc, argv,"hs:l:t:")) != -1) 
  	{		
  	switch (option) 
  	{		
  		case 'h' :		
           printf("Usage executable -s {no. of slave processes to be spawned} -l {log file name} -t {time in seconds when master will terminate with all children} \n");		
           return 1;
           break;	
		case 's':
    	   slaveProcess = atoi(optarg);
    	   if(slaveProcess < 1 || slaveProcess >20)
    	   {
    	   		fprintf(stderr, "Slave processes count out of range, continuing with default value 5" );
    	   		slaveProcess = 5; // default value
    	   }

           break;
        case 'l':
        	logfile = optarg;
        	break;
        case 't':
        	ztime = atoi(optarg);
        	if (ztime <= 0)
        	{
        		fprintf(stderr, "Time should be greater than 0, continuing with default value 20" );
        		ztime = 20; //default value
        	}
        	break;
        case '?':
        	if (optopt == 'l')
        	{
        		fprintf(stderr,"Option -%c requires an argument (log filename). \n", optopt);
        		//logfile = "log.txt";
        	}
        	else if (optopt == 's')
        	{
        		fprintf(stderr,"Option -%c requires an argument (slave count).\n", optopt);
        		//slaveProcess = 5;
        	}

        	else if (optopt == 't')
        	{
        		fprintf(stderr,"Option -%c requires an argument (execution time).\n", optopt);
        		//ztime = 20;
        	}
        	else if (isprint (optopt))
        	{
        		fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        		return -1;
        	}
        	else
        		fprintf(stderr,"Usage executable -s {no. of slave processes to be spawned} -l {log file name}-t {time in seconds when master will terminate with all children} \n");
        	return -1;

		}

	}
//signal handling 
	signal(SIGINT, interruptHandler); 
	signal(SIGALRM, interruptHandler);
	alarm(ztime);	

	key = 555;
	//Create shared memory segment 
	shmid = shmget(key, 200*sizeof(shinfo), 0744 |IPC_CREAT |IPC_EXCL);
	if ((shmid == -1) && (errno != EEXIST)) /* real error */
	{
		perror("Unable to create shared memory");
		return -1;
	}
	if (shmid == -1)
	{
		printf("Shared Memory Already created");
		return -1;
	}
	else
	{
		shinfo = (shmClock*)shmat(shmid,NULL,0);
		if (shinfo == (void*)-1)
			return -1;
		// clock initially set to 0
		shinfo->nsec = 0;
		shinfo->sec = 0;
	}
// Open log file 
FILE *fp = fopen(logfile, "a");
	spawnSlaveProcess(slaveProcess);



return 0;
}

void spawnSlaveProcess(int noOfSlaves)
{
	int i;
	for(i = 0; i < noOfSlaves; i++) 
	{
		
    	printf("Creating Slave process : %d \n", spawnedSlaves);
    	childpid = fork();
    if(childpid < 0) 
    	{
      		printf("Failed to fork ");
      		break;
    	}
    if (childpid == 0)
	    {
    	//execl user.c
	    	gpid = getpgrp();
			printf("exec %d\n",i);    
    	}
    	spawnedSlaves++;
	}
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

