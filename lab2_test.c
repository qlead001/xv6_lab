#include "types.h"
#include "user.h"

#define XSTR(s) STR(s)
#define STR(s) #s

#define	MAX	16
#define	MIN	1
#define DEFAULT	1

int main(int argc, char *argv[])
{
	
	int PScheduler(void);

  printf(1, "\n This program tests the correctness of your lab#2\n");
  
	PScheduler();
	return 0;
 }
  
    
     int PScheduler(void){
		 
    // use this part to test the priority scheduler. Assuming that the
    // priorities range between range between MIN and MAX
    // MAX is the highest priority and MIN is the lowest priority.  

  int pid;
  int i,j,k;
  
    printf(1, "\n  Step 2: testing the priority scheduler and setprior"
              "(int priority) system call:\n");
    printf(1, "\n  Step 2: Assuming that the priorities range between"
              " range between "XSTR(MIN)" to "XSTR(MAX)"\n");
    printf(1, "\n  Step 2: "XSTR(MAX)" is the highest priority. All processes"
              " have a default priority of "XSTR(DEFAULT)"\n");
    printf(1, "\n  Step 2: The parent processes will switch to priority "
              XSTR(MAX)"\n");
    setprior(MAX);
    for (i = 0; i <  3; i++) {
	pid = fork();
	if (pid > 0 ) {
		continue;}
	else if ( pid == 0) {

		setprior(MIN+5*i);	
		for (j=0;j<50000;j++) {
			for(k=0;k<1000;k++) {
				asm("nop"); }}
		printf(1, "\n child# %d with priority %d has finished! \n",
                       getpid(),MIN+5*i);		
		exit(0);
        }
        else {
			printf(2," \n Error \n");
			
        }
	}

	if(pid > 0) {
		for (i = 0; i <  3; i++) {
			wait(0);

		}
                     printf(1,"\n if processes with highest priority finished"
                              " first then its correct \n");
}
	exit(0);		
	return 0;}
