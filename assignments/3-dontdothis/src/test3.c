#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile int done;

void handler(int signal, siginfo_t *siginfo, void *context){
	printf("signal %d was caught\n", signal);
	
	printf("your UID is %d\n", siginfo->si_uid);
	printf("your PID is %d\n", siginfo->si_pid);

	done = 1;
}

int main() {
	struct sigaction sa;
	
	int pid = getpid();
	
	printf("Ok, lets go - kill me (%d).\n", pid);
	
	/*Use a more elaborated sigaction handler*/
	
	sa.sa_sigaction = handler; //tell the program that we have
												 //our own sigaction handlar called 'handler'
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	
	if(sigaction(SIGINT, &sa, NULL) == -1){
	fprintf(stderr, "something happed\n");
		return(1);
	}
	
	while(!done){
	}
	
	printf("Told you so!\n");
	return(0);
}

