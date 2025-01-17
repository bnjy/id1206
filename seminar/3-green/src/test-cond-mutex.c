#include <stdlib.h>
#include <stdio.h>
#include "green.h"
//#include "green-new.h"
#include "queue.h"

int flag = 0;
green_cond_t cond;
green_mutex_t mutex;

void *test(void *arg){
	int id = *(int*)arg;
	int loop = 10000;
	while(loop>0){
		green_mutex_lock(&mutex);
		while(flag != id){
			//green_mutex_unlock(&mutex);
			printf("thread %d: %d\n", id, loop);
			green_cond_wait_mutex(&cond, &mutex);
		}
		flag = (id + 1) % 2;
		green_cond_signal(&cond);
		green_mutex_unlock(&mutex);
		loop--;
	}
	return NULL;
}


int main(){
	green_t g0, g1;
	
	//thread identifier
	int a0 = 0;
	int a1 = 1;
	
	green_cond_init(&cond);
	green_mutex_init(&mutex);
	green_create(&g0, test, &a0);
	green_create(&g1, test, &a1);
	
	green_join(&g0, NULL);
	green_join(&g1, NULL);
	
	printf("done\n");
	return 0;
}
