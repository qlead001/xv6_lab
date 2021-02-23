#include "types.h"

#include "user.h"

void
spin(int n)
{
  int i,j;
  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      asm("nop");
    }
  }
}

int main(int argc, char * argv[]) {

  int PScheduler(void);
  int priorAge(void);
  int timePrint(void);

  printf(1, "\nThis program tests the correctness of your lab#2\n");

  if(atoi(argv[1]) == 1)
    PScheduler();
  else if(atoi(argv[1]) == 2)
    priorAge();
  else if(atoi(argv[1]) == 3)
    timePrint();
  else 
    printf(1, "\ntype \"lab2_test 1\" to test priority scheduling, "
              "\"lab2_test 2\" to test process aging and printing "
              "turnaround/wait times, and \"lab2_test 3\" to test "
              "turnaround/wait times\n");

  exit(0);
  return 0;
}

int PScheduler(void) {

  // use this part to test the priority scheduler. Assuming that the
  // priorities range between range between MIN_PRIOR and MAX_PRIOR
  // MAX_PRIOR is the highest priority and MIN_PRIOR is the lowest priority.  

  int pid;
  int i, j, k;

  printf(1, "\n\tStep 1: testing the priority scheduler and setprior"
            "(int priority) system call:\n");
  printf(1, "\n\tStep 2: Assuming that the priorities range between"
            " range between %d to %d\n", MIN_PRIOR, MAX_PRIOR);
  printf(1, "\n\tStep 2: %d is the highest priority. All processes"
            " have a default priority of %d\n", MAX_PRIOR, DEF_PRIOR);
  printf(1, "\n\tStep 2: The parent processes will switch to priority %d"
            "\n", MAX_PRIOR);
  setprior(MAX_PRIOR);
  for (i = 0; i < 3; i++) {
    pid = fork();
    if (pid > 0) {
      continue;
    } else if (pid == 0) {

      setprior(MIN_PRIOR + 5 * i);
      for (j = 0; j < 50000; j++) {
        for (k = 0; k < 1000; k++) {
          asm("nop");
        }
      }
      printf(1, "\n child# %d with priority %d has finished! \n",
             getpid(), MIN_PRIOR + 5 * i);
      exit(0);
    } else {
      printf(2, " \n Error \n");

    }
  }

  if (pid > 0) {
    for (i = 0; i < 3; i++) {
      wait(0);

    }
    printf(1, "\n if processes with highest priority finished"
              " first then its correct \n");
  }
  exit(0);
  return 0;
}

int priorAge(void){
  int pid, start, end;
  struct times t;

  printf(1, "\n\tTest 2: Create a child and set its priority to %d\n", MIN_PRIOR);
  printf(1, "\n\tTest 2: Parent will have a priority of %d\n", MAX_PRIOR-1);
  printf(1, "\n\tTest 2: Child should run debug to show priority increase\n");
  printf(1, "\n\tTest 2: Before exiting the child will print turnaround/wait times\n\n");

  setprior(MAX_PRIOR-1);
  pid = fork();
  if (pid > 0) {
    start = uptime();
    while(!waitpid(pid, 0, 1));
    end = uptime();
    printf(1, "Parent waited %d ticks which should be similar to child turnaround time\n", end-start);
    printf(1, "\nChild's wait and turnaround times should be similar because it is waiting for its priority to increase\n");
  } else if (pid == 0) {
    debug();
    setprior(MIN_PRIOR);
    debug();
    gettimes(&t);
    printf(1, "\nChild:\n\tTurnaround Time: %d\n\tWait Time: %d\n", t.turnaround, t.waittime);
    exit(0);
  }

  exit(0);
  return 0;
}

int timePrint(void){
  int start, end;
  struct times t;

  printf(1, "\n\tTest 3: Sleep for a while and busy loop for a while\n");
  printf(1, "\n\tTest 3: Then print turnaround/wait times\n");

  printf(1, "\nSleeping...");
  start = uptime();
  sleep(500);
  end = uptime();
  printf(1, " slept for %d ticks\n", end-start);

  printf(1, "\nSpinning...");
  start = uptime();
  spin(50000);
  end = uptime();
  printf(1, " spun for %d ticks\n", end-start);

  gettimes(&t);
  printf(1, "\nTurnaround Time: %d\nWait Time: %d\n", t.turnaround, t.waittime);

  exit(0);
  return 0;
}
