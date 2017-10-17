typedef struct osClock {
    unsigned long sec;
    unsigned long nsec;
}shmClock;


typedef struct Msg{
  
  char msgText[1000];
} shmMsg;
