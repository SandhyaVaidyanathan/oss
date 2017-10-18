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
pid_t childpid;
int shmid, Mshmid;
shmClock *shinfo;
shmMsg *shmsginfo;
time_t startTime;
char* arg1;
int status = 0;

const int ZTIME_DEFAULT = 20;
const int MAXSLAVE_DEFAULT = 5;
const int TOTALPROCESS = 100;
const unsigned long int NANOSECOND = 1000000000; 
const int TIMEINC = 6000;

void spawnSlaveProcess(int);
void interruptHandler(int);
void clearSharedMem1();
void clearSharedMem2();

int main(int argc, char const *argv[])
{
	char* logfile;
	logfile = "log.txt";
	int option = 0;	
	int slaveProcess = 0,ztime = 0 ;

	key_t clock_key, msg_key;

	arg1 = (char*)malloc(40);

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

	clock_key = 555;
	msg_key = 666;
	//Create shared memory segment 
	shmid = shmget(clock_key, 20*sizeof(shinfo), 0766 |IPC_CREAT |IPC_EXCL);
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


	//Shared memory for shmmsg

	//Create shared memory segment 
	Mshmid = shmget(msg_key, 20*sizeof(shmsginfo), 0766 |IPC_CREAT |IPC_EXCL);
	if ((Mshmid == -1) && (errno != EEXIST)) /* real error */
	{
		perror("Unable to create shared memory");
		return -1;
	}
	if (Mshmid == -1)
	{
		printf("Shared Memory Already created");
		return -1;
	}
	else
	{
		shmsginfo = (shmMsg*)shmat(Mshmid,NULL,0);
		if (shmsginfo == (void*)-1)
			return -1;
		// clock initially set to 0
		shmsginfo->term_ns = 0;
		shmsginfo->term_s = 0;
		shmsginfo->process_id =-1;
	}

// Open log file 

FILE *fp = fopen(logfile, "a");

	spawnSlaveProcess(slaveProcess);

	// start clock
	fprintf(stderr, "Starting the clock..\n" );
	  startTime = time(NULL);
	  /*This process continues until 2 seconds have passed in the simulated system time, 100 processes
		in total have been generated or the executable has been running for the maximum time allotted */
	  while(shinfo->sec < 2  && spawnedSlaves <= TOTALPROCESS)
        {
        	if ( time(NULL) <(startTime + ztime))
        	{
        		shinfo->nsec += TIMEINC;
        	if (shinfo->nsec > (NANOSECOND - 1))
            {
            shinfo->nsec -= (NANOSECOND - 1);
            	shinfo->sec++;
            }
        	}

        	if (shmsginfo->process_id!= -1)
        	{
        		printf("Master: Child %d is terminating at my time %lu.%lu because it reached %lu.%lu in slave \n",
        		 shmsginfo->process_id, shinfo->sec, shinfo->nsec, shmsginfo->term_s,shmsginfo->term_ns);
        		fprintf(fp, "Master: Child %d is terminating at my time %lu.%lu because it reached %lu.%lu in slave \n",
        		 shmsginfo->process_id, shinfo->sec, shinfo->nsec, shmsginfo->term_s,shmsginfo->term_ns);
        		shmsginfo->process_id = -1;
        		shmsginfo->term_ns = 0;
        		shmsginfo->term_s = 0;
     			//Spawn 1 more process if less than 100
     			if (spawnedSlaves < TOTALPROCESS)
     			{
     				spawnSlaveProcess(1);
     			}
        	} 
        }

//clearing memory
free(arg1);
wait(&status);
clearSharedMem1();
clearSharedMem2();

fclose(fp);
kill(-getpgrp(),SIGQUIT);

return 0;
}

void spawnSlaveProcess(int noOfSlaves)
{
	int i;
	//Forking processes
	for(i = 0; i < noOfSlaves; i++) 
	{ 
       	if((childpid = fork())<=0)
       		break;
    }

    if (childpid == 0)
	    {
    	//execl user.c    	
			fprintf(stderr,"exec %d\n",i);  
			sprintf(arg1, "%d", i);
			//Calling user.c program
			execl("user", arg1, NULL); 
    	}
    	spawnedSlaves++;
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

  kill(-getpgrp(), 9);
  clearSharedMem1();
  clearSharedMem2();
}

void clearSharedMem1()
{
	int error = 0;
	if(shmdt(shinfo) == -1) {
		error = errno;
	}
	if((shmctl(shmid, IPC_RMID, NULL) == -1) && !error) {
		error = errno;
	}
	if(!error) {
		return ;
	}
}
void clearSharedMem2()
{
	int error = 0;
	if(shmdt(shmsginfo) == -1) {
		error = errno;
	}
	if((shmctl(Mshmid, IPC_RMID, NULL) == -1) && !error) {
		error = errno;
	}
	if(!error) {
		return ;
	}
}

