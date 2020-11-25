#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(){

	char *heap = malloc(20);

	printf("The heap variable at adress: %p\n", &heap);
	printf("Pointing to: %p\n", heap);
	
	int pid = getpid();
	
	printf("\n\n /proc/%d/maps \n\n", pid); //prints the system command
	char command[50];
	sprintf(command, "cat /proc/%d/maps", pid);
	system(command);
	
	return 0;
	
}
