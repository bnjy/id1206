#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int volatile count;

void handler(int signal) {
	printf("signal %d ouch that hurt\n", signal);
	count++;
}

int main() {
	struct sigaction sa;
	
	int pid = getpid();
	
	printf("ok, lets go, kill me (%d) if you can!\n", pid);
	
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	
	if(sigaction(SIGINT, &sa, NULL) != 0) {
	return(1);
	}
	
	while(count!= 4){
	}
	
	printf("i've had enough!\n");
	return(0);
} 
