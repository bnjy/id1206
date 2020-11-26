#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0
#define TAKENSTRUCT 0

#ifdef TAKENSTRUCT

#define HEAD (sizeof(struct taken))
#define MIN(size) (((size)>(16))?(size):(16))
#define MAGIC(memory) ((struct taken*)memory - 1)
#define HIDE(block) (void*)((struct taken*)block + 1)

#else

#define HEAD (sizeof(struct head))
#define MIN(size) (((size)>(8))?(size):(8))
#define MAGIC(memory) ((struct head*)memory - 1)
#define HIDE(block) (void*)((struct head*)block + 1)

#endif

#define LIMIT(size) (MIN(0) + HEAD + size)
#define ALIGN 8
#define ARENA (64*1024)

struct taken {
	uint16_t bfree; // 2 bytes, the status of the block before
	uint16_t bsize; // 2 bytes, the size of the block before 
	uint16_t free;  // 2 bytes, the status of the block
	uint16_t size;  // 2 bytes, the size (max 2^16 i.e. 64 Kbyte)
};

struct head {
	uint16_t bfree; // 2 bytes, the status of the block before
	uint16_t bsize; // 2 bytes, the size of the block before 
	uint16_t free;  // 2 bytes, the status of the block
	uint16_t size;  // 2 bytes, the size (max 2^16 i.e. 64 Kbyte)
	struct head *next; // 8 bytes pointer
	struct head *prev; // 8 bytes pointer
};

struct head *after(struct head *block){
	return (struct head*)((char *)block + (block->size + HEAD));
}

struct head *before(struct head *block){
	return (struct head*)((char *)block - block->bsize - HEAD);
}

struct head *split(struct head *block, int size){
	int rsize = block->size - size - HEAD;
	block->size = rsize;
	
	struct head *split = after(block);
	split->bsize = block->size; // or block->size ? same same
	split->bfree = block->free; //FALSE; // block is not freed from freeList, so we set FALSE. // or block->free ? same same			
	split->size = size;
	split->free = FALSE; // also our split block is set to FALSE
	split->size = size;
	
	struct head *aft = after(split); 
	aft->bsize = split->size;
	
	return split;
}

struct head *arena = NULL;

struct head *new(){
	if(arena != NULL){
		printf("one arena already allocated.\n");
		return NULL;
	}
	
	/* using mmap, but could have used sbrk()
		 extended implementation to handle larger blocks need other
		 parameters
	*/
	struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	
	if(new == MAP_FAILED){
		printf("mmap failed: error %d\n", errno);
		return NULL;
	}
		
	/* make room for head and dummy */
	unsigned int size = ARENA - 2*HEAD;

	new->bfree = FALSE; //prevent from trying to merge with a non existing block?
	new->bsize = 0;
	new->free = TRUE;
	new->size = size;
	
	struct head *sentinel = after(new);
	
	/* only touch the status fields */
	sentinel->bfree = new->free;
	sentinel->bsize = size;
	sentinel->free = FALSE;
	sentinel->size = 0;
	
	/* this is the only arena we have */
	arena = (struct head*)new;
	return new;
}

struct head *flist; // = NULL;

void detach(struct head *block){
	// change next only if the block to be detached is not the last
	// block in the list 
	if(block->next != NULL){
		block->next->prev = block->prev;
	}
	// change prev only if the block to be detached is not the first
	// block in the list 
	if(block->prev != NULL){
		block->prev->next = block->next;
	}
	// else point to next block in freeList	
	else {
		//flist = flist->next;
		flist = block->next;
		//block->next->prev = NULL;
	}
}

void insert(struct head *block){
	block->next = flist; // insert at front of the list
	block->prev = NULL;
	
	//if there are already blocks in the freeList, point it's previous
	//first block to the new block
	if(flist != NULL){
		flist->prev = block;
	}
	flist = block;
}

/* adjust the block request so it is a even multiple of ALIGN and not smaller than MIN, e.g. 8 bytes */
int adjust(size_t size){

	// will either return 8, if argument is < 8. Otherwise it
	// will return the size that we passed to the function.
	int newSize = MIN(size);
	
	if(newSize % ALIGN == 0){
		return newSize;
	}else{
		int rem = newSize % ALIGN;
		// add ALIGN to make sure we never return a smaller block
		// than requested
		return newSize - rem + ALIGN;
	}
}

struct head *find(int size){
	struct head *freeList = flist; // get the freeList
	while(freeList != NULL){ 			 // loop the freeLists
		if(freeList->size >= size){
			detach(freeList);
			// can we split the block?
			if(freeList->size >= LIMIT(size)){
				struct head *block = split(freeList, size);
				insert(freeList);
				struct head *aft = after(block);
				aft->bfree = FALSE;
				block->free = FALSE;
				//aft->bsize = block->size;
				//insert(freeList);
				return block;
			} else {
				freeList->free = FALSE;
				struct head *aft = after(freeList); 
				aft->bfree = FALSE;
				return freeList;
			}
		} else {
			freeList = freeList->next;
		}
	}
	return NULL;
}

struct head *merge(struct head *block){
	
	struct head *aft = after(block);
	
	if(block->bfree){
		// unlink the block before
		struct head *bef = before(block);
		detach(bef);
		
		// calculate and set the total size of the merged block
		bef->size = bef->size + block->size + HEAD;
		 
		// update the block after the merged blocks
		aft->bsize = bef->size;
		
		// continue with the merged block
		block = bef;
	}
	
	if(aft->free){
		// unlink the block
		detach(aft);
		
		// calculate and set the total size of merged blocks
		block->size = block->size + aft->size + HEAD;
		
		// update the block after the merged block
		aft = after(block); //jump to the block after after(block)
		aft->bsize = block->size;
	}
	return block;
}

void *dalloc(size_t request){
	if(request <= 0){
		return NULL;
	}
	int size = adjust(request);
	struct head *taken = find(size);
	if(taken == NULL){
		return NULL;
	} else{
		return HIDE(taken);
	}
}

void dfree(void *memory){
	if(memory != NULL){
		struct head *block = (struct head*) MAGIC(memory); //memory;
		//block = merge(block);
		printf("Trying to free adress: %p\n", MAGIC(memory));
		
		struct head *aft = after(block);
		block->free = TRUE;
		aft->bfree = block->free;
		insert(block);
	}
	return;
}

void init(){
	struct head *first = new();
	insert(first);
}

void traverseWithArena(){
	struct head *temp = arena;
	while(temp->size != 0){
		printf("adress: %p, free: %d, size: %d, bfree: %d, bsize: %d, prev adress: %p\n", temp, temp->free, temp->size, temp->bfree, temp->bsize, before(temp));
		temp = after(temp);
	}
}

void traverse(struct head *arena){
	struct head *temp = arena;
	struct head *freeList = after(arena);
	while(freeList != NULL && freeList->size != 0){
		printf("adress: %p\t, free: %d\t, size: %d\t, bfree: %d\t, bsize: %d\tss\n", freeList, freeList->free, freeList->size, freeList->bfree, freeList->bsize);
		freeList = after(freeList);
	}
}

int lengthOfFree(){
	int i = 0;
	struct head *freeList = flist;
	while(freeList != NULL){
		i++;
		freeList = freeList->next;
	}
	return i;
}

void sizes(int *buffer, int max){
	struct head *next = flist;
	int i = 0;
	while((next != NULL) & (i < max)){
		buffer[i] = next->size;
		i++;
		next = next->next;
	}
}

void flistLengthAndAverageSize(int amount){
	struct head *temp = flist;
	int length = 0;
	int totSize = 0;
	while(temp != NULL){
		length++;
		totSize += temp->size;
		temp = temp->next;
	}
	
	int average = totSize/length;
	
	printf("Allocs\tfreeList length\n");
	printf("%d\t%d\n", amount, length);
	printf("Average block size: %d\n", average);
	printf("\n");
}

void reset(){
	arena = NULL;
	flist = NULL;
}

void sanity(){
	printf("Sanity check\n");	
	struct head *head = flist; //after(arena);
	struct head *prev = head->prev;

	int size = head->size;
	
	while(head != NULL && head->size != 0){
		if(head->prev != prev){
			printf("block in flist has wrong prev\n");
			exit(1);
		}
		if(head->free != TRUE){
			printf("block in flist is not free\n");
			exit(1);
		}
		if((head->size % ALIGN) != 0){
			printf("block size in flist do not align\n");
			exit(1);
		}
		prev = head;
		head = head->next;
	}
	printf("Sanity check OK\n");
}
/*
void testFunction(){
	struct head *first = new(); //init
	insert(first); //init
	
	struct head *var1 = dalloc(100);
	struct head *var2 = dalloc(200);
	struct head *var3 = dalloc(1000);
	struct head *var4 = dalloc(100);
	dfree(var1);
	dfree(var2);
	dfree(var3);
	dfree(var4);
	//traverseWithArena();
	traverse(first);
	sanity();
}

int main(){
	testFunction();
}
*/


