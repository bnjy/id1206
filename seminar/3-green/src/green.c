#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include "green.h"
#include "queue.h"

#define TRUE 1
#define FALSE 0

#define STACK_SIZE 4096

#define PERIOD 100

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, NULL, FALSE};

static green_t *running = &main_green;

static sigset_t block;
void timer_handler(int);

struct queue_t *ready_queue = NULL;
struct queue_t *susp_queue = NULL;

/*
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
	
	// init timer interupts
	sigemptyset(&block); // init set (&block) of signal, given in argument
	// SIGVTALRM indicates expiration of a timer that measures CPU
	// time used by the current process.
	sigaddset(&block, SIGVTALRM); // add signal to set
	struct sigaction act = {0}; // init sigaction
	struct timeval interval;
	struct itimerval period;
	act.sa_handler = timer_handler;
	assert(sigaction(SIGVTALRM, &act, NULL) == 0); //take the signal SIGVTALRM and reference the sigaction
	interval.tv_sec = 0;
	interval.tv_usec = PERIOD;
	period.it_interval = interval;
	period.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &period, NULL);
}

/*
When the alarm goes off, timer_handler function will be called (according to initialization of signal in function init()). The timer_handler will suspend the running thread and schedule the next one in line in the
*/
void timer_handler(int sig){
	sigprocmask(SIG_BLOCK, &block, NULL); // block our set of signals, no interrupts below
	green_t *susp = running;
	
	// add the running to the ready queue
	queue_insert(ready_queue, susp);
	
	//find the next thread for execution
	green_t *next = ready_queue->head;
	running = next;
	queue_remove(ready_queue);
	
	sigprocmask(SIG_UNBLOCK, &block, NULL); // unblock out set of signals
	
	swapcontext(susp->context, next->context);
}

//replaced with queue struct, to handle suspended ssqueue easier
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
	sigprocmask(SIG_BLOCK, &block, NULL);
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
	sigprocmask(SIG_UNBLOCK, &block, NULL);
}

int green_create(green_t *new, void *(*fun)(void*), void *arg){
	int *id = (int*)malloc(sizeof(int));
	*id = *(int*)arg;
	
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

/*
Puts the current running thread in the ready_queue, and swaps to the
thread first in the queue.
*/
int green_yield(){
	//catch the running thread
	green_t *susp = running;
	
	//add susp to ready queue
	queue_insert(ready_queue, susp);
	
	//select the next thread for execution
	green_t *next = ready_queue->head;
	running = next;
	queue_remove(ready_queue);
	//swap context to new thread
	swapcontext(susp->context, next->context);
	
	return 0;
}

/*
The join operation will wait for a thread to terminate. The running thread will
be added to the join field, and then another thread is selected for execution.
*/
int green_join(green_t *thread, void **res){
	sigprocmask(SIG_BLOCK, &block, NULL);
	
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
	
	sigprocmask(SIG_UNBLOCK, &block, NULL);
	
	return 0;
}

/*    Conditional variables     */
void green_cond_init(green_cond_t *cond){
	cond->queue = init_queue();
}

/* if running thread has to wait for other thread,
	 suspend and queue it again.
*/
void green_cond_wait(green_cond_t *cond){
	sigprocmask(SIG_BLOCK, &block, NULL);
	
	green_t *susp = running;
	queue_insert(cond->queue, susp);
	
	green_t *next = queue_remove(ready_queue);
	running = next;
	swapcontext(susp->context, running->context);
	
	sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_signal(green_cond_t *cond){
	sigprocmask(SIG_BLOCK, &block, NULL);
	
	//if(queue_remove(cond->queue) == NULL) return;
	green_t *init = queue_remove(cond->queue);
	queue_insert(ready_queue, init);
	
	sigprocmask(SIG_UNBLOCK, &block, NULL);
}

void green_cond_wait_mutex(green_cond_t *cond, green_mutex_t *mutex){
	// block timer interrupt
	sigprocmask(SIG_BLOCK, &block, NULL);
	
	// "if running thread has to wait for other thread,
	// suspend and queue it again."
	//suspend the running thread on the condition
	green_t *susp = running;
	queue_insert(cond->queue, susp);
	
	/* mutex */
	if(mutex != NULL){
		//release the lock if we have a mutex
		mutex->taken = FALSE;
		//move suspended thread to ready queue
		green_t *ready = queue_remove(mutex->queue);//new
		queue_insert(ready_queue, susp);
	}
	
	//schedule the next thread
	green_t *next = queue_remove(ready_queue);
	running = next;
	swapcontext(susp->context, next->context);
	
	if(mutex != NULL){
		//try to take the lock
		if(mutex->taken){
			// bad luck, mutex already taken, suspend
			susp = running;
			queue_insert(ready_queue, susp);
			green_t *next = queue_remove(ready_queue);
			running = next;
			swapcontext(susp->context, next->context);
		} else {
			// take the lock
			mutex->taken = TRUE;
		}
	}
	sigprocmask(SIG_UNBLOCK, &block, NULL);
}

/*           MUTEX           */
int green_mutex_init(green_mutex_t *mutex){
	mutex->taken = FALSE;
	mutex->susp = NULL;
	mutex->queue = init_queue();
	return 0;
}

int green_mutex_lock(green_mutex_t *mutex){
	//block timer interrupt
	sigprocmask(SIG_BLOCK, &block, NULL);
	
	green_t *susp = running;
	if(mutex->taken){
		//suspend the running thread
		//mutex->susp = susp;
		queue_insert(mutex->queue, susp); //new	
		//find the next thread
		green_t *next = ready_queue->head;
		running = next;
		queue_remove(ready_queue);
		swapcontext(susp->context, next->context);
	} else {
		//take the lock
		mutex->taken = TRUE;
	}
	//unblock
	sigprocmask(SIG_UNBLOCK, &block, NULL);
	return 0;
}

int green_mutex_unlock(green_mutex_t *mutex){
	//block timer interrupt
	sigprocmask(SIG_BLOCK, &block, NULL);
	
	if(mutex->susp != NULL) {
		// move suspended thread to ready queue
		green_t *next = mutex->queue->head;
		running = next;
		queue_remove(mutex->queue);
		queue_insert(ready_queue, next);
	} else {
		// release lock
		mutex->taken = FALSE;
		mutex->susp = NULL;
	}
	//unblock
	sigprocmask(SIG_UNBLOCK, &block, NULL);
	
	return 0;
}


