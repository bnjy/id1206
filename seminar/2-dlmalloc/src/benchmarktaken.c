#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "rand.h"
#include "dlmall.h"

#define BUFFER 100
#define SIZES 1000
#define ROUNDS 100000
#define LOOP 1000
#define BLOCKS 1000

int main(){
	int *array[BLOCKS];
	
	init();
	
	clock_t begin = clock();
	
	for(int i = 0; i < BLOCKS; i++){
		array[i] = dalloc(16);
	}
	for(int i = 0; i < ROUNDS; i++){
		for(int i = 0; i < LOOP; i++){
			*array[i] = 100;
		}
	}
	clock_t end = clock();
	double time_spent = (double)(end-begin) / CLOCKS_PER_SEC;
	printf("Time spent: %f\n", time_spent);
}
/*
int freq[SIZES];

int cmp(const void *a, const void *b){
	return *(int*)a - *(int *)b;
}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("number of allocations as arg \n");
	}
	
	int allocations = atoi(argv[1]);
	char *name = argv[2];
	
	init();
	
	// init a buffer with pointers
	void *buffer[BUFFER];
	for(int i = 0; i < BUFFER; i++){
		buffer[i] = NULL;
	}
	
	for(int i = 0; i < allocations; i++){
		int index = rand() % BUFFER;
		if(buffer[index] != NULL){
			dfree(buffer[index]);
		}
			size_t size = (size_t)request();
			int *memory;
			memory = dalloc(size);
			
			buffer[index] = memory;
			*memory = 123;
		}
		
		FILE *file = fopen(name, "w");
		
		sizes(freq, SIZES);
		int length = lengthOfFree();
		qsort(freq, length, sizeof(int), cmp);
		
		for(int i = 0; i < length; i++){
			fprintf(file, "%d\n", freq[i]);
		}

		fclose(file);

		printf("Length of the freeList: %d\n", length);
		//printf("Sum: %d\n", sum);
		//printf("Average size: %d\n", average);
		
		return 0;
}
*/
