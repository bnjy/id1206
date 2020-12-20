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

//extern struct queue_t *ready_queue;

typedef struct green_cond_t {
	struct queue_t *queue;
} green_cond_t;

int green_create(green_t *thread, void *(*fun)(void*), void *args);
int green_yield();
int green_join(green_t *thread, void** val);
void green_thread(void);
void green_cond_init(green_cond_t *cond);
void green_cond_wait(green_cond_t *cond);
void green_cond_signal(green_cond_t *cond);

#endif
