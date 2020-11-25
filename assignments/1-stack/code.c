#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

char global[] = "This is a global string";
const int read_only = 123456;

int main(){
	
	int pid = getpid();
	
	long unsigned int p = 0x1;
	
	printf("long unsigned int	\"p\" with value (0x%lx): %p \n",p, &p);
	
	printf("\n\n /proc/%d/maps", pid);
	char command[50];
	sprintf(command, "cat /proc/%d/maps", pid);
	system(command);
	
	printf("\n");
	
	return 0;

}
