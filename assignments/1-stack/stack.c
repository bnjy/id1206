#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

//method zot
// får en minnesadress från foo, som nu variablen stop pekar på
void zot(unsigned long *stop){
	
	// sätt ett värde på r
	unsigned long int r = 0x3;
	
	//deklarera en pekare 'i', pekar ingenstans
	unsigned long int *i;
	
	printf(" i's minnesadress		värdet av i \n");
	
	//loopa
	for(i = &r; i <= stop; i++){
	
		// %p = print adress of pointer
		//
		printf(" %p			0x%lx\n", i, *i);
	}
}

//ta emot en minnesadress som variablen stop sedan pekar på
void foo(unsigned long int *stop, int dummy1, int dummy2, int dummy3, int dummy4, int dummy5, int dummy6, int dummy7, int dummy8, int dummy9){

	// sätt ett värde på q
	unsigned long int q = 0x2;
	
	//kalla på zot med minneasadressen till variablen stop
	zot(stop);
}

int main(){
	
	int pid = getpid();
	
	// sätt ett värde på p
	unsigned long int p = 0x1;
	
	//skicka 'p's minnesadress till foo 
	foo(&p, 10, 10, 10, 10, 10, 10, 10, 10, 10);
	
back:
	printf("p: %p \n", &p); //prints memory adress of p
	printf("back: %p \n", &&back); //prints memory adress of back
	
	printf("\n\n /proc/%d/maps \n\n", pid); //prints the system command
	char command[50];
	sprintf(command, "cat /proc/%d/maps", pid);
	system(command);
	
	return 0;
	
}

