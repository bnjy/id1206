#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

void handler(int signal){
	printf("signal %d was caught\n", signal);
	exit(1);
	return;
}

int not_so_good(){
	int x = 0;
	return 1 % x;
}

int main() {
	struct sigaction sa;
	
	printf("Ok, lets go - i'll catch my own error.\n");
	
	sa.sa_handler = handler; //tell the program that we have
													 //our own sigaction handlar called "handler"
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	
	/*and now we catch... FPE signals*/
	sigaction(SIGFPE, &sa, NULL);
	
	not_so_good();
	
	printf("Will probably not write this.\n");
	return(0);
}

