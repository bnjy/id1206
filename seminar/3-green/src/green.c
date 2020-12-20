#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"
#include "queue.h"

#define TRUE 1
#define FALSE 0

#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;

struct queue_t *ready_queue = NULL;
struct queue_t *susp_queue = NULL;
/*
static green_t *ready_queue = NULL;
static green_t *head = NULL;
static green_t *tail = NULL;
*/
/* Init main context when program is loaded,
	 so the running thread is properly initialized
 */
static void init() __attribute__((constructor));
void init(){
	getcontext(&main_cntx);
	ready_queue = init_queue();
}

//replaced with queue struct, to handle susp_queue easier
/*
void queue_insert(green_t *new){
	// If queue is empty, set both head and tail to single thread
	if(head == NULL){
		head = new;
		tail = new;
	}else{ // else place thread in back of the queue
		tail->next = new;
		tail = new;
	}
}

green_t *queue_remove(){
	// If queue is empty, return null.
	if(head == NULL){
		return NULL;
	}else{ // else init new thread "thread" and point to head.
		green_t *thread = head;
		if(head == tail){ // if only one element in queue, dereference tail
			tail = NULL;
		}
	head = head->next;
	thread->next = NULL;
	return thread;
	}
}
*/
void green_thread(){
	green_t *this = running;
	
	void *result = (*this->fun)(this->arg);
	
	// place waiting (joining) thread in ready queue
	if(this->join != NULL){
		queue_insert(ready_queue, this->join);
	}
	
	// save result of execution
	this->retval = result;
		
	// we're a zombie
	this->zombie = TRUE;
	
	// find the next thread to run
	green_t *next = ready_queue->head;
	running = next;
	queue_remove(ready_queue);
	setcontext(next->context);
}

int green_create(green_t *new, void *(*fun)(void*), void *arg){
	int *id = (int*)malloc(sizeof(int));
	*id = *(int*)arg;
	printf("create thread %d\n", *id);
	
	ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(cntx);
	
	/* Allocate a stack for the thread */
	void *stack = malloc(STACK_SIZE);
	
	/* Set stack pointer for threads context */
	cntx->uc_stack.ss_sp = stack;
	cntx->uc_stack.ss_size = STACK_SIZE;
	makecontext(cntx, green_thread, 0);
	
	/* Set the new green_t struct variables */
	new->context = cntx;
	new->fun = fun;
	new->arg = arg;
	new->next = NULL;
	new->join = NULL;
	new->retval = NULL;
	new->zombie = FALSE;
	
	/* Insert new thread to ready queue */
	queue_insert(ready_queue, new);
	
	return 0;
}

int green_yield(){
	green_t *susp = running;
	
	//add susp to ready queue
	queue_insert(ready_queue, susp);
	
	//select the next thread for execution
	green_t *next = ready_queue->head;
	running = next;
	queue_remove(ready_queue);
	swapcontext(susp->context, next->context);
	return 0;
}

int green_join(green_t *thread, void **res){
	if(!thread->zombie){
		green_t *susp = running;
		
		//add as joining thread
		thread->join = susp;
		
		//select the next thread for execution
		green_t *next = ready_queue->head;
		running = next;
		queue_remove(ready_queue);
		swapcontext(susp->context, next->context);
	}
	//collect result
	if(thread->retval != NULL){
		*res = thread->retval;
	}
	//free context
	free(thread->context);
	printf("Memory freed.\n");
	
	return 0;
}

/*    Conditional variables     */
/* DEQUEUE EXAMPLE

		green_t *next = ready_queue->head;
		running = next;
		queue_remove(ready_queue);
		swapcontext(susp->context, next->context);
*/
/* ENQUEUE EXAMPLE

	queue_insert(ready_queue, susp);
*/
void green_cond_init(green_cond_t *cond){
	cond->queue = init_queue();
}

/* if running thread has to wait for other thread,
	 suspend and queue it again.
*/
void green_cond_wait(green_cond_t *cond){
	green_t *susp = running;
	
	queue_insert(cond->queue, susp);
	
	green_t *next = queue_remove(ready_queue);
	running = next;
	swapcontext(susp->context, running->context);
}

void green_cond_signal(green_cond_t *cond){
	//if(queue_remove(cond->queue) == NULL) return;
	green_t *init = queue_remove(cond->queue);
	queue_insert(ready_queue, init);
}


