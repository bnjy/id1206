#ifndef _GREEN
#define _GREEN

#include <ucontext.h>

typedef struct green_t {
	ucontext_t *context;
	void *(*fun)(void*);
	void *arg;
	struct green_t *next;
	struct green_t *join;
	void *retval;
	int zombie; //indicates if the thread has terminated or not
} green_t;

typedef struct green_cond_t {
	struct green_t *susp_list;
} green_cond_t;

typedef struct green_mutex_t {
	volatile int taken;
	//handle the list
	struct green_t *susp;
} green_mutex_t;

/* Mutex locks */
int green_mutex_init(green_mutex_t *mutex);
int green_mutex_lock(green_mutex_t *mutex);
int green_mutex_unlock(green_mutex_t *mutex);

/* create, yield, join */
int green_create(green_t *thread, void *(*fun)(void*), void *args);
int green_yield();
int green_join(green_t *thread, void** val);
void green_thread(void);

/* Conditional variables */
void green_cond_init(green_cond_t *cond);
void green_cond_wait(green_cond_t *cond);
void green_cond_signal(green_cond_t *cond);
void green_cond_wait_mutex(green_cond_t *cond, green_mutex_t *mutex);

/* other */
green_t *queue_remove(green_t **list);
void queue_insert(green_t **list, green_t *thread);
void timer_handler(int);

#endif
