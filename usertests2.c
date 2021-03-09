#include "fcntl.h"
#include "types.h"
#include "stat.h"
#include "user.h"

#define COLOUR_ON
#include "colours.h"

const int stdout = 1;
const int stderr = 2;

struct {
  int tests;
  int passes;
  int errors;
  int warnings;
  const char* testname;
} stats;

int pidtable[MAX_PRIOR-MIN_PRIOR];

void
print_stats(void){
  printf(stdout, "\n");

  if(stats.warnings > 0)
    printf(stdout, OUT_YELLOW"Total Warnings: %d"OUT_RESET
                   "\n", stats.warnings);
  if(stats.errors > 0)
    printf(stdout, OUT_RED"Total Errors: %d"OUT_RESET"\n", stats.errors);

  if(stats.passes == stats.tests)
    printf(stdout, OUT_GREEN"Passed all tests"OUT_RESET"\n");
  else
    printf(stdout, "Passed %d out of %d tests\n", stats.passes, stats.tests);
}

void
test_start(const char* name){
  printf(stdout, OUT_REV"%s test"OUT_RESET"\n", name);
  stats.tests++;
  stats.testname = name;
}

void
test_pass(void){
  printf(stdout, OUT_GREEN OUT_REV"Success: %s ok"OUT_RESET"\n", stats.testname);
  stats.passes++;
}

void
test_fail(void){
  printf(stdout, OUT_RED OUT_REV"Failure: %s failed"OUT_RESET"\n", stats.testname);
}

void
print_warn(const char* str){
  printf(stderr, OUT_YELLOW"Warning: %s"OUT_RESET"\n", str);
  stats.warnings++;
}

void
print_error(const char* str){
  printf(stderr, OUT_RED"Error: %s"OUT_RESET"\n", str);
  stats.errors++;
}

void
fail(const char* str){
  printf(stderr, OUT_RED"Error: %s"OUT_RESET"\n", str);
  stats.errors++;
  exit(E_ERR | ((stats.warnings>0)*E_WARN));
}

void
exit_child(){
  int warns, errors;
  warns = ((stats.warnings>0)*E_WARN);
  errors = ((stats.errors>0)*E_ERR);
  exit(warns | errors);
}

void
warn_expect(const char* str, int ret, int exp){
  printf(stderr, OUT_YELLOW"Warning: %s\n", str);
  printf(stderr, "\tReturned: "OUT_BOLD"%d"OUT_RESET
                 OUT_YELLOW"\n\tExpected: "OUT_BOLD"%d"
                 OUT_RESET"\n", ret, exp);
  stats.warnings++;
}

void
error_expect(const char* str, int ret, int exp){
  printf(stderr, OUT_RED"Error: %s\n", str);
  printf(stderr, "\tReturned: "OUT_BOLD"%d"OUT_RESET
                 OUT_RED"\n\tExpected: "OUT_BOLD"%d"
                 OUT_RESET"\n", ret, exp);
  stats.errors++;
}

int
wait_stat(int* status){
  int out, stat;
  if(!status)
    status = &stat;
  out = wait(status);
  if(*status & E_ERR)
    stats.errors++;
  if(*status & E_WARN)
    stats.warnings++;
  return out;
}

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

// Returns 1 if two values are within threshold
// Otherwise it returns 0
int
thresh(int a, int b, int thresh)
{
  int diff;
  diff = (a > b) ? a - b : b - a;
  return (diff < thresh);
}

// does gettimes accurately report times?
void
gettimetest(void)
{
  test_start("gettime");

  int pid, start, end, init_time, i, count, killed, waited = 0, failed = 0;
  struct times t;

  gettimes(&t);
  init_time = uptime() - t.turnaround;

  for(i = 0; i < 10; i++){
    count = killed = 0;

    start = uptime();
    sleep(100+i*10);
    waited += uptime() - start;

    pid = fork();
    if(pid < 0)
      fail("fork failed");

    if(!pid){
      sleep(100+i*10);
      exit(E_FINE);
    }

    while(!waitpid(pid, 0, 1)){
      count++;
      if(count > 1e9 && !killed){
        print_error("Child process failed to increase priority");
        printf(stdout, "Killing child process %d...\n", pid);
  
        if(kill(pid) == -1)
          fail("kill failed");
        killed = 1;
        failed++;
      }
    }
    if(killed)
      continue;

    gettimes(&t);
    end = uptime();
    if(!thresh(t.turnaround, end - init_time, 10)){
      error_expect("Turnaround time incorrect", t.turnaround, end - init_time);
      failed++;
    }
    if(!thresh(t.waittime, waited, 10)){
      error_expect("Wait time incorrect", t.waittime, waited);
      failed++;
    }
  }

  if(failed)
    test_fail();
  else
    test_pass();
}

// do children processes inherit the parent's priority?
void
priorforktest(void)
{
  test_start("prior fork");

  int prior, pid, out, failed=0;

  for(prior = MIN_PRIOR; prior <= MAX_PRIOR; prior++){
    if(setprior(prior) == -1)
      fail("setprior failed");

    pid = fork();
    if(pid < 0)
      fail("fork failed");

    if(!pid){
      out = setprior(prior);
      if(out == -1)
        fail("setprior failed");

      if(out != prior){
        error_expect("Child has unexpected priority", out, prior);
        exit_child();
      }
      exit_child();
    }
  }

  while(wait_stat(&out) != -1)
    if(out & E_ERR)
      failed++;

  if(failed){
    printf(stdout, OUT_RED"Error: %d of %d children with incorrect priorities"
                   OUT_RESET"\n", failed, MAX_PRIOR-MIN_PRIOR+1);
    stats.errors++;
    test_fail();
  }else{
    test_pass();
  }
}

// does process aging increase priority?
void
prioragetest(void)
{
  test_start("prior age");

  int pid, count = 0, killed = 0;

  if(setprior(MAX_PRIOR-1) == -1)
    fail("setprior failed");

  pid = fork();
  if(pid < 0)
    fail("fork failed");

  if(!pid){
    setprior(MIN_PRIOR);
    exit(E_FINE);
  }

  while(!waitpid(pid, 0, 1)){
    count++;
    if(count > 1e9 && !killed){
      print_error("Child process failed to increase priority");
      printf(stdout, "Killing child process %d...\n", pid);

      if(kill(pid) == -1)
        fail("kill failed");
      killed = 1;
    }
  }

  if(killed)
    test_fail();
  else
    test_pass();
}

// does setprior correctly set priority?
void
setpriortest(void)
{
  test_start("setprior");

  int prior, priorOut, failed = 0;

  if(setprior(MAX_PRIOR+1) != -1){
    print_error("setprior did not fail on priority greater than max");
    failed++;
  }

  if(setprior(MIN_PRIOR-1) != -1){
    print_error("setprior did not fail on priority less than min");
    failed++;
  }

  if(setprior(MIN_PRIOR) == -1)
    fail("setprior failed");

  for(prior = MIN_PRIOR+1; prior <= MAX_PRIOR; prior++){
    if((priorOut = setprior(prior)) != prior-1){
      error_expect("setprior unexpected value", priorOut, prior-1);
      failed++;
    }
  }

  if((priorOut = setprior(MAX_PRIOR)) != MAX_PRIOR){
    error_expect("setprior unexpected value", priorOut, MAX_PRIOR);
    failed++;
  }

  if(failed)
    test_fail();
  else
    test_pass();
}

// does the scheduler prioritise correctly?
void
priortest(void)
{
  test_start("prior");

  int prior, pidOut, failed = 0;

  if(setprior(MAX_PRIOR) == -1)
    fail("setprior failed");

  for(prior = MIN_PRIOR; prior <= MAX_PRIOR-1; prior++){
    pidtable[prior-MIN_PRIOR] = fork();
    if(pidtable[prior-MIN_PRIOR] < 0)
      fail("fork failed");

    if(!pidtable[prior-MIN_PRIOR]){
      if(setprior(prior) == -1)
        fail("setprior failed");
      spin(100);
      exit(E_FINE);
    }
  }

  for(prior = MAX_PRIOR-1; prior >= MIN_PRIOR; prior--){
    pidOut = wait_stat(0);
    if(pidOut < 0)
      fail("wait failed");

    if(pidOut != pidtable[prior-MIN_PRIOR])
      failed++;
  }

  if(failed){
    printf(stdout, OUT_YELLOW"%d of %d processes out of order "
                   "(only passes with 1 cpu)"OUT_RESET"\n",
                   failed, MAX_PRIOR-MIN_PRIOR);
    stats.warnings++;
    test_fail();
  }else{
    test_pass();
  }
}

// does self waiting fail?
void
selfwaittest(void)
{
  test_start("self wait");

  if(waitpid(getpid(), 0, 1) != -1){
    print_error("waitpid should fail on self wait");
    test_fail();
    return;
  }

  test_pass();
}


// does nohang work when using waitpid?
void
nohangtest(void)
{
  test_start("nohang");

  int pid, output = 0, count = 0, killed = 0;

  if(setprior(MIN_PRIOR+1) == -1)
    fail("setprior failed");

  pid = fork();
  if(pid < 0)
    fail("fork failed");

  if(!pid){
    spin(100);
    exit(E_FINE);
  }

  while(!output){
    output = waitpid(pid, 0, 1);
    if(output < 0)
      fail("waitpid failed");

    count++;

    if(count > 1e5 && !killed){
      print_warn("Child process never finished");
      printf(stdout, "Killing process %d\n", pid);
      if(kill(pid) == -1)
        fail("kill failed");

      if(setprior(MIN_PRIOR) == -1)
        fail("setprior failed");

      killed = 1;
    }
  }

  if(count < 5){
    print_error("waitpid with nohang still waited");
    test_fail();
  }else{
    test_pass();
  }
}

// does waitpid wait properly?
void
waitpidtest(void)
{
  test_start("waitpid");

  int pid1, pid2, out, failed = 0;
  int waitstatus;
  int exitstatus;

  for(exitstatus = 0; exitstatus < 10; exitstatus++){
    pid1 = fork();
    if(pid1 < 0)
      fail("fork failed");

    if(!pid1)
      exit(exitstatus);

    pid2 = fork();
    if(pid2 < 0)
      fail("fork failed");

    if(!pid2){
      if(waitpid(pid1, &waitstatus, 0) != pid1){
        print_error("waitpid failed");
        exit(-1);
      }
      if(waitstatus != exitstatus){
        print_error("waitpid wrong status");
        exit(-1);
      }
      if(waitpid(pid1, 0, 0) != -1){
        print_error("waitpid should have failed but did not");
        exit(-1);
      }
      exit(2*exitstatus);
    }

    if((out = waitpid(pid2, &waitstatus, 0)) == -1)
      fail("waitpid failed");

    if(out != pid2){
      error_expect("waitpid returned incorrect pid", out, pid2);
      failed++;
    }

    if(waitstatus == -1)
      stats.errors++;

    if(waitstatus != 2*exitstatus){
      error_expect("waitpid wrong status", waitstatus, 2*exitstatus);
      failed++;
    }
    if(wait(0) != -1) {
      print_error("Waitpid failed to reap a child");
      failed++;
    }
  }

  if(failed)
    test_fail();
  else
    test_pass();
}

// does exit status get returned by wait?
void
exitstatustest(void)
{
  test_start("exit status");

  int pid, out, failed = 0;
  int waitstatus;
  int exitstatus;

  for(exitstatus = 0; exitstatus < 10; exitstatus++){
    pid = fork();
    if(pid < 0)
      fail("fork failed");

    if(pid){
      out = wait(&waitstatus);
      if(out == -1)
        fail("wait failed");

      if(out != pid){
        error_expect("wait wrong pid", out, pid);
        failed++;
        continue;
      }

      if(waitstatus != exitstatus){
        error_expect("wait wrong status", waitstatus, exitstatus);
        failed++;
        continue;
      }
    } else {
      exit(exitstatus);
    }
  }

  if(failed)
    test_fail();
  else
    test_pass();
}

void
testfunc(void){
  test_start("sample");
  printf(stdout, "Warnings = %d\tErrors = %d\n",stats.warnings,stats.errors);
  print_warn("test warning");
  print_error("test error");
  warn_expect("test did not return normally", 17, 28);
  error_expect("test did not return normally", 17, 28);
  printf(stdout, "Warnings = %d\tErrors = %d\n",stats.warnings,stats.errors);
  int pid = fork();
  if(!pid){
    warn_expect("Child did not return normally", 999, 0);
    fail("Child failure");
  }else{
    wait_stat(0);
  }
  printf(stdout, "Warnings = %d\tErrors = %d\n",stats.warnings,stats.errors);
  test_pass();
  test_start("new attempt");
  test_fail();
  test_start("another one");
  test_pass();
  fail("Critical failure");
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
      exit(E_WARN);
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
  gettimetest();

  //testfunc();
  print_stats();

  exit(E_FINE);
}
