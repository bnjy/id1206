#include <stdlib.h>
#include <stdio.h>
#include "green.h"
//#include "queue.h"

green_mutex_t mutex;

/*
Test only using the mutex locks. No cond, yield, yada yada.
*/
void *test(void *arg){
	int id = *(int*)arg;
	int loop = 10;
	while(loop>0){
		green_mutex_lock(&mutex);
		printf("thread %d: %d\n", id, loop);
		loop--;
		green_mutex_unlock(&mutex);
	}
	return NULL;
}

int main(){
	printf("start\n");
	green_t g0, g1;
	
	//thread identifier
	int a0 = 0;
	int a1 = 1;
	
	green_mutex_init(&mutex);
	
	green_create(&g0, test, &a0);
	green_create(&g1, test, &a1);
	
	green_join(&g0, NULL);
	green_join(&g1, NULL);
	
	printf("done\n");
	return 0;
}
