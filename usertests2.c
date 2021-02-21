#include "fcntl.h"
#include "types.h"
#include "stat.h"
#include "user.h"

const int stdout = 1;
const int stderr = 2;

int pidtable[MAX_PRIOR-MIN_PRIOR];

// Does nothing n^2 times
// Helper function for priortest()
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

// do children processes inherit the parent's priority?
void
priorforktest(void)
{
  printf(stdout, "prior fork test\n");

  int prior, pid, out, failed=0;

  for(prior = MIN_PRIOR; prior <= MAX_PRIOR; prior++){
    if(setprior(prior) == -1){
      printf(stdout, "setprior failed\n");
      exit(E_ERR);
    }

    pid = fork();
    if(pid < 0){
      printf(stdout, "fork failed\n");
      exit(E_ERR);
    }

    if(!pid){
      out = setprior(prior);
      if(out == -1){
        printf(stdout, "setprior failed\n");
        exit(E_ERR);
      }
      if(out != prior){
        printf(stdout, "Child %d has priority %d expected %d\n",
               getpid(), out, prior);
        exit(E_ERR);
      }
      exit(E_FINE);
    }
  }

  while(wait(&out) != -1)
    if(out == E_ERR)
      failed++;

  if(failed)
    printf(stdout, "prior fork test failed with %d of %d children with"
                   " incorrect priorities\n", failed, MAX_PRIOR-MIN_PRIOR+1);
  else
    printf(stdout, "prior test ok\n");
}

// does process aging increase priority?
void
prioragetest(void)
{
  printf(stdout, "prior age test\n");

  int pid, count = 0, killed = 0;

  if(setprior(MAX_PRIOR-1) == -1){
    printf(stdout, "setprior failed\n");
    sleep(10);
    exit(E_ERR);
  }

  pid = fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    exit(E_ERR);
  }

  if(!pid){
    setprior(MIN_PRIOR);
    exit(E_FINE);
  }

  while(!waitpid(pid, 0, 1)){
    count++;
    if(count > 100000 && !killed){
      printf(stdout, "Child process failed to increase priority\n");
      printf(stdout, "Killing child process...\n");

      if(kill(pid) == -1){
        printf(stdout, "kill failed\n");
        exit(E_ERR);
      }
      killed = 1;
    }
  }

  if(killed){
    printf(stdout, "prior test failed\n");
    exit(E_ERR);
  }

  printf(stdout, "prior age test ok\n");
}

// does setprior correctly set priority?
void
setpriortest(void)
{
  printf(stdout, "setprior test\n");

  int prior, priorOut;

  if(setprior(MAX_PRIOR+1) != -1){
    printf(stdout, "setprior did not fail on priority greater than max\n");
    exit(E_ERR);
  }

  if(setprior(MIN_PRIOR-1) != -1){
    printf(stdout, "setprior did not fail on priority less than min\n");
    exit(E_ERR);
  }

  if(setprior(MIN_PRIOR) == -1){
    printf(stdout, "setprior failed\n");
    exit(E_ERR);
  }

  for(prior = MIN_PRIOR+1; prior <= MAX_PRIOR; prior++){
    if((priorOut = setprior(prior)) != prior-1){
      printf(stdout, "setprior returned %d expected %d\n", priorOut, prior-1);
      exit(E_ERR);
    }
  }

  if((priorOut = setprior(MAX_PRIOR)) != MAX_PRIOR){
    printf(stdout, "setprior returned %d expected %d\n", priorOut, MAX_PRIOR);
    exit(E_ERR);
  }

  printf(stdout, "setprior test ok\n");
}

// does the scheduler prioritise correctly?
void
priortest(void)
{
  printf(stdout, "prior test\n");

  int prior, pidOut, failed=0;

  if(setprior(MAX_PRIOR) == -1){
    printf(stdout, "setprior failed\n");
    exit(E_ERR);
  }

  for(prior = MIN_PRIOR; prior <= MAX_PRIOR-1; prior++){
    pidtable[prior-MIN_PRIOR] = fork();
    if(pidtable[prior-MIN_PRIOR] < 0){
      printf(stdout, "fork failed\n");
      exit(E_ERR);
    }
    if(!pidtable[prior-MIN_PRIOR]){
      if(setprior(prior) == -1){
        printf(stdout, "setprior failed\n");
        exit(E_ERR);
      }
      spin(100);
      exit(E_FINE);
    }
  }

  for(prior = MAX_PRIOR-1; prior >= MIN_PRIOR; prior--){
    pidOut = wait(0);
    if(pidOut < 0){
      printf(stdout, "wait failed\n");
      exit(E_ERR);
    }
    if(pidOut != pidtable[prior-MIN_PRIOR])
      failed++;
  }

  if(failed)
    printf(stdout, "prior test failed with %d of %d processes out of order "
                   "(only passes with 1 cpu)\n",
                   failed, MAX_PRIOR-MIN_PRIOR);
  else
    printf(stdout, "prior test ok\n");
}

// does self waiting fail?
void
selfwaittest(void)
{
  printf(stdout, "self wait test\n");

  if(waitpid(getpid(), 0, 1) != -1){
    printf(stdout, "waitpid should fail on self wait\n");
    exit(E_ERR);
  }

  printf(stdout, "self wait ok\n");
}


// does nohang work when using waitpid?
void
nohangtest(void)
{
  printf(stdout, "nohang test\n");

  int pid, output = 0, count = 0, killed = 0;

  if(setprior(MIN_PRIOR+1) == -1){
    printf(stdout, "setprior failed\n");
    exit(E_ERR);
  }

  pid = fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    exit(E_ERR);
  }
  if(!pid){
    spin(100);
    exit(E_FINE);
  }

  while(!output){
    output = waitpid(pid, 0, 1);
    if(output < 0){
      printf(stdout, "waitpid failed\n");
      exit(E_ERR);
    }
    count++;

    if(count > 10000 && !killed){
      printf(stdout, "Warning: child process never finished, "
                     "killing process %d\n", pid);
      if(kill(pid) == -1){
        printf(stdout, "kill failed\n");
        exit(E_ERR);
      }

      if(setprior(MIN_PRIOR) == -1){
        printf(stdout, "setprior failed\n");
        exit(E_ERR);
      }
      killed = 1;
    }
  }

  if(count < 5){
    printf(stdout, "waitpid with nohang still waited\n");
    exit(E_ERR);
  }

  printf(stdout, "nohang ok\n");
}

// does waitpid wait properly?
void
waitpidtest(void)
{
  printf(stdout, "waitpid test\n");

  int pid1, pid2;
  int waitstatus;
  int exitstatus;

  for(exitstatus = 0; exitstatus < 10; exitstatus++){
    pid1 = fork();
    if(pid1 < 0){
      printf(stdout, "fork failed\n");
      exit(E_ERR);
    }
    if(!pid1)
      exit(exitstatus);

    pid2 = fork();
    if(pid2 < 0){
      printf(stdout, "fork failed\n");
      exit(-1);
    }
    if(!pid2){
      if(waitpid(pid1, &waitstatus, 0) != pid1){
        printf(stdout, "waitpid failed\n");
        exit(-1);
      }
      if(waitstatus != exitstatus){
        printf(stdout, "waitpid wrong status\n");
        exit(-1);
      }
      if(waitpid(pid1, 0, 0) != -1){
        printf(stdout, "waitpid should have failed but didn't\n");
        exit(-1);
      }
      exit(2*exitstatus);
    }

    if(waitpid(pid2, &waitstatus, 0) != pid2){
      printf(stdout, "waitpid failed\n");
      exit(E_ERR);
    }
    if(waitstatus != 2*exitstatus){
      printf(stdout, "waitpid wrong status\n");
      exit(E_ERR);
    }
    if(wait(0) != -1) {
      printf(stdout, "Waitpid failed to reap a child\n");
      exit(E_ERR);
    }
  }

  printf(stdout, "waitpid ok\n");
}

// does exit status get returned by wait?
void
exitstatustest(void)
{
  printf(stdout, "exit status test\n");

  int pid;
  int waitstatus;
  int exitstatus;

  for(exitstatus = 0; exitstatus < 10; exitstatus++){
    pid = fork();
    if(pid < 0){
      printf(stdout, "fork failed\n");
      exit(E_ERR);
    }
    if(pid){
      if(wait(&waitstatus) != pid){
        printf(stdout, "wait wrong pid\n");
        exit(E_ERR);
      }
      if(waitstatus != exitstatus){
        printf(stdout, "wait wrong status\n");
        exit(E_ERR);
      }
    } else {
      exit(exitstatus);
    }
  }

  printf(stdout, "exit status ok\n");
}

int
main(int argc, char *argv[])
{
  printf(1, "usertests2 starting\n");

  int force = (argc > 1 && strcmp(argv[1], "-f") == 0);

  if(open("usertests2.ran", 0) >= 0){
    if(!force){
      printf(stdout, "already ran user tests 2 --"
             " rebuild fs.img or specify -f\n");
      exit(2);
    }else{
      printf(stdout, "already ran user tests 2 -- re-running\n");
    }
  }else{
    close(open("usertests2.ran", O_CREATE));
  }

  selfwaittest();
  exitstatustest();
  waitpidtest();

  nohangtest();

  setpriortest();
  priortest();
  priorforktest();
  prioragetest();

  exit(E_FINE);
}
