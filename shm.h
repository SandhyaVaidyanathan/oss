typedef struct osClock {
    unsigned long sec;
    unsigned long nsec;
}shmClock;

typedef struct Msg{
  unsigned long int term_s;
 unsigned long int term_ns;
  int process_id;
} shmMsg;

