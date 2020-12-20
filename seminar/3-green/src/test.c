#include <stdlib.h>
#include <stdio.h>
#include "green.h"

void *test(void *arg){
	int i = *(int*)arg;
	int loop = 4;
	while(loop>0){
		printf("thread %d: %d\n", i, loop);
		loop--;
		green_yield();
	}
}

void *test2(void *arg){
	int *ret = (int*)malloc(sizeof(int));
	*ret = *(int*)arg + 3;
	printf("thread %d: %d\n", *(int*)arg, *ret);
	return ret;
}

int main(){
	printf("start\n");
	green_t g0, g1, g2;
	
	//thread identifier
	int a0 = 0;
	int a1 = 1;
	int a2 = 2;
	
	int *retvalB;
	
	green_create(&g0, test, &a0);
	green_create(&g1, test, &a1);
	green_create(&g2, test2, &a2);
	
	green_join(&g0, NULL);
	green_join(&g1, NULL);
	green_join(&g2, (void **)&retvalB);
	
	printf("retvalB: %d\n", *retvalB);
	free(retvalB);
	
	printf("done\n");
	return 0;
}
