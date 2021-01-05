#include <stdlib.h>
#include <stdio.h>
#include "green.h"
//#include "queue.h"

green_cond_t cond;

/*
Test only using the timer interupts. No cond or yield.
*/
void *test(void *arg){
	int id = *(int*)arg;
	int loop = 4;
	while(loop>0){
		printf("thread %d: %d\n", id, loop);
		loop--;
	}
	return NULL;
}

int main(){
	green_t g0, g1;
	
	//thread identifier
	int a0 = 0;
	int a1 = 1;
	
	//green_cond_init(&cond);
	
	green_create(&g0, test, &a0);
	green_create(&g1, test, &a1);
	
	green_join(&g0, NULL);
	green_join(&g1, NULL);
	
	printf("done\n");
	return 0;
}
