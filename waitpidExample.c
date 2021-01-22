// Example of waitpid in action

#include "types.h"
#include "user.h"

int
main(void)
{
  int ticks = 300;

  int pid1, pid2, curpid, waitedPid, status;

  curpid = getpid();

  // Create first child
  printf(1, "[%d] Forking...\n", curpid);
  pid1 = fork();
  if(pid1 == -1){
    printf(1, "[Error:%d] Fork failed\n", curpid);
    exit(1);
  }

  // First child sleeps then exits
  if(pid1 == 0){
    curpid = getpid();
    printf(1, "[%d] Sleeping for %d ticks...\n", curpid, ticks);
    sleep(ticks);
    printf(1, "[%d] Exiting...\n", curpid);
    exit(0);
  }

  // Create second child
  printf(1, "[%d] Forking...\n", curpid);
  pid2 = fork();
  if(pid2 == -1){
    printf(1, "[Error:%d] Fork failed\n", curpid);
    exit(1);
  }

  // Second child waits for first child then exits
  if(pid2 == 0){
    curpid = getpid();
    printf(1, "[%d] Waiting for process %d...\n", curpid, pid1);
    waitedPid = waitpid(pid1, 0, 0);
    if(waitedPid == -1){
      printf(1, "[Error:%d] Waitpid failed\n", curpid);
      exit(1);
    } else if(waitedPid != pid1){
      printf(1, "[Error:%d] Waitpid returned %d, expected %d\n",
             curpid, waitedPid, pid1);
      exit(1);
    }
    printf(1, "[%d] Done waiting, exiting...\n", curpid);
    exit(0);
  }

  // Parent waits for second child
  waitedPid = waitpid(pid2, &status, 0);
  if(waitedPid == -1){
    printf(1, "[Error:%d] Waitpid failed\n", curpid);
    exit(1);
  } else if(waitedPid != pid2){
    printf(1, "[Error:%d] Waitpid returned %d, expected %d\n",
           curpid, waitedPid, pid2);
    exit(1);
  }
  printf(1, "[%d] Process %d exited with status %d\n",
         curpid, waitedPid, status);

  // Parent checks if it has anymore children
  waitedPid = wait(0);
  if(waitedPid != -1){ // We should not have children
    printf(1, "[Error:%d] Unexpected child process %d exited\n",
           curpid, waitedPid);
    exit(1);
  }
  printf(1, "[%d] No more children, exiting...\n", curpid);

  exit(0);
}
