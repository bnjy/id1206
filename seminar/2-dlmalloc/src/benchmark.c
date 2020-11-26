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
#define BLOCKS 2000


int freq[SIZES];

int cmp(const void *a, const void *b){
	return *(int*)a - *(int *)b;
}

void bench(int allocations, char *name){

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
			size_t size = 16; //(size_t)request();
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
/*
		int sum;
		for(int i = 0; i < length-1; i++){
			sum += freq[i];
			printf("%d\n", freq[i]);
		}
		
		int average = (sum/length);
		*/
		printf("Length of the freeList: %d\n", length);
		//printf("Sum: %d\n", sum);
		//printf("Average size: %d\n", average);

}

void bench16(int size){

	clock_t start, end;
	int *array[BLOCKS];
	
	init();
	
	start = clock();
	for(int i = 0; i < BLOCKS; i++){
		array[i] = dalloc(size);
	}
	
	for(int i = 0; i < ROUNDS; i++){
		for(int j = 0; j < BLOCKS; j++){
			//int *memory = array[i];
			//*memory = j;
			*array[j] = 100;
		}
	}
	end = clock();
	printf("Total time: %d\n", (int)(end-start));
}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("number of allocations as arg \n");
	}
	
	int first = atoi(argv[1]);
	char *name = argv[2];
	
	//bench(first, name);
	bench16(first);
	
	return 0;
}
