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
#include <sys/msg.h>
#include <getopt.h>

#include "shm.h"

int spawnedSlaves = 0;
const int MAXSLAVE = 20;


void spawnSlaveProcess(int);

int main(int argc, char const *argv[])
{
	char* logfile;
	logfile = "log.txt";
	int option = 0;	
	int slaveProcess = 0,ztime = 0 ;
	int shmid;

	pid_t childpid;


	if (argc < 2){ // check for valid number of command-line arguments		
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
        		fprintf(stderr,"Invalid Usage:");
        		fprintf(stderr,"Option -%c requires an argument (log filename). \n", optopt);
        		logfile = "log.txt";
        	}
        	else if (optopt == 's')
        	{
        		fprintf(stderr,"Invalid Usage:");
        		fprintf(stderr,"Option -%c requires an argument (slave count).\n", optopt);
        		slaveProcess = 5;
        	}

        	else if (optopt == 't')
        	{
        		fprintf(stderr,"Invalid Usage:");
        		fprintf(stderr,"Option -%c requires an argument (execution time).\n", optopt);
        		ztime = 20;
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

// Open log file 
FILE *fp = fopen(logfile, "a");
return 0;
}

void spawnSlaveProcess(int noOfSlaves)
{
	int i;
	for(i = 0; i < count; i++) 
	{
    	printf("Spawning process %d \n.", spawnedSlaves);

    if((childpid = fork()) < 0) {
      perror("Failed to fork ");

    	}
	}
}
