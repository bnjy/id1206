#include <stdlib.h>
#include <unistd.h>

/* datastructure chunk
	 hold size and a next.
 */
struct chunk {
	int size;
	struct chunk *nextPointer;
};

struct chunk *freeList = NULL; //create a chunk called freeList

void *malloc(size_t size){
	if(size == 0){
		return NULL;
	}
	struct chunk *next = freeList;
	struct chunk *prev = NULL;
	
	while(next != NULL){
		if(next->size >= size){
			if(prev != NULL){
				prev->nextPointer = next->nextPointer;
			} else {
				freeList = next->nextPointer;
			}
			return (void*)(next+1);
		} else{
			prev = next;
			next = next->nextPointer;
		}
	}
	
	/* use sbrk() to allocate new memory */
	void *memory = sbrk(size + sizeof(struct chunk));
	if(memory == (void *)-1) {
		return NULL;
	} else {
	struct chunk *cnk = (struct chunk*)memory;
	cnk->size = size;
		return (void *)(cnk + 1);
	}
}

void free(void *memory) {
	if(memory != NULL) {
		/* we are jumping back one chunk position */
		//cast memory to chunk pointer, which cast to chunk pointer to 
		//match cnk declaration
		struct chunk *cnk = (struct chunk*)((struct chunk*)memory - 1);
		cnk->nextPointer = freeList;
		freeList = cnk;
	}
	return;
}
