#include "fcntl.h"
#include "types.h"
#include "stat.h"
#include "user.h"

const int stdout = 1;

#define	E_FINE	0
#define	E_ERR	1

#define	PRIOR_MAX	16
#define	PRIOR_MIN	1
#define	PRIOR_DEF	1
int pidtable[PRIOR_MAX-PRIOR_MIN];

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

// does setprior correctly set priority?
void
setpriortest(void)
{
  printf(stdout, "setprior test\n");

  int prior, priorOut;

  if(setprior(PRIOR_DEF) != PRIOR_DEF){
    printf(stdout, "setprior did not return expected default priority\n");
    exit(E_ERR);
  }

  if(setprior(PRIOR_MAX+1) != -1){
    printf(stdout, "setprior did not fail on priority greater than max\n");
    exit(E_ERR);
  }

  if(setprior(PRIOR_MIN-1) != -1){
    printf(stdout, "setprior did not fail on priority less than min\n");
    exit(E_ERR);
  }

  if(setprior(PRIOR_MIN) != PRIOR_DEF){
    printf(stdout, "setprior did not return expected default priority\n");
    exit(E_ERR);
  }

  for(prior = PRIOR_MIN+1; prior <= PRIOR_MAX; prior++){
    if((priorOut = setprior(prior)) != prior-1){
      printf(stdout, "setprior returned %d expected %d\n", priorOut, prior-1);
      exit(E_ERR);
    }
  }

  if((priorOut = setprior(PRIOR_MAX)) != PRIOR_MAX){
    printf(stdout, "setprior returned %d expected %d\n", priorOut, PRIOR_MAX);
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

  setprior(PRIOR_MAX);

  for(prior = PRIOR_MIN; prior <= PRIOR_MAX-1; prior++){
    pidtable[prior-PRIOR_MIN] = fork();
    if(pidtable[prior-PRIOR_MIN] < 0){
      printf(stdout, "fork failed\n");
      exit(E_ERR);
    }
    if(!pidtable[prior-PRIOR_MIN]){
      setprior(prior);
      spin(100);
      exit(E_FINE);
    }
  }

  for(prior = PRIOR_MAX-1; prior >= PRIOR_MIN; prior--){
    pidOut = wait(0);
    if(pidOut < 0){
      printf(stdout, "wait failed\n");
      exit(E_ERR);
    }
    if(pidOut != pidtable[prior-PRIOR_MIN])
      failed++;
  }

  if(failed)
    printf(stdout, "prior test failed with %d of %d processes out of order "
                   "(only passes with 1 cpu)\n", failed, PRIOR_MAX-PRIOR_MIN);
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

  setprior(PRIOR_MIN+1);

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
      setprior(PRIOR_MIN);
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

  if(open("usertests2.ran", 0) >= 0){
    printf(1, "already ran user tests 2 -- rebuild fs.img\n");
    exit(2);
  }
  close(open("usertests2.ran", O_CREATE));

  selfwaittest();
  exitstatustest();
  waitpidtest();

  setpriortest();
  priortest();

  nohangtest();

  exit(E_FINE);
}
