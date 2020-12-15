#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"

#define TRUE 1
#define FALSE 0

#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;
static green_t *head = NULL;
static green_t *tail = NULL;

static void init() __attribute__((constructor));

void init(){
	getcontext(&main_cntx);
}

void queue_insert(green_t *new){
	if(head == 0){
		head = new;
		tail = new;
	}else{
		new->next = head;
		head = new;
	}
}

green_t *queue_remove(){
	if(head == NULL){
		return NULL;
	}else{
		green_t *thread = head;
		if(head == tail){
			tail = NULL;
		}
	head = head->next;
	thread->next = NULL;
	return thread;
	}
}

int green_create(green_t *new, void *(*fun)(void*), void *arg){
	ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(cntx);
	
	void *stack = malloc(STACK_SIZE);
	
	cntx->uc_stack.ss_sp = stack;
	cntx->uc_stack.ss_size = STACK_SIZE;
	//makecontext(cntx, green_thread, 0);
	
	new->context = cntx;
	new->fun = fun;
	new->arg = arg;
	new->next = NULL;
	new->join = NULL;
	new->retval = NULL;
	new->zombie = FALSE;
	
	queue_insert(new);
}

/*

typedef struct green_t {
	ucontext_t *context;
	void *(*fun)(void*);
	void *arg;
	struct green_t *next;
	struct green_t *join;
	void *retval;
	int zombie; //indicates if the thread has terminated or not
} green_t;

*/

void green_thread(){
	green_t *this = running;
	
	void *result = (*this->fun)(this->arg);
	
	// place waiting (joining) thread in ready queue
	
	
	// save result of execution
	
		
	// we're a zombie
	
	
	// find the next thread to run
	
	
}

void *funcA(void *arg){
	printf("A\n");
	//printf("%d\n", queue_size);
}

void *funcB(void *arg){
	printf("B\n");
	//printf("%d\n", queue_size);
}

void *funcC(void *arg){
	printf("C\n");
	//printf("%d\n", queue_size);
}

void test_queue(){
	green_t *a, *b, *c;
	
	a = (green_t*)malloc(sizeof(green_t));
	b = (green_t*)malloc(sizeof(green_t));
	c = (green_t*)malloc(sizeof(green_t));
	
	a->fun = funcA;
	b->fun = funcB;
	c->fun = funcC;
	
	a->next = b;
	b->next = c;

	queue_insert(a);
	queue_insert(b);
	queue_insert(c);
	
	green_t *next = queue_remove();
	next = queue_remove();
	next = queue_remove();
	next = queue_remove();
}

int main(){
	test_queue();
}
